#include "Krystal.Booey/Editing/VisiblePosition.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLogicalOrderTraversal.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LineSelection.hpp"
#include "Krystal.Booey/Rendering/CaretRectComputation.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include <algorithm>
#include <unicode/utf8.h>

namespace krys::boo
{
  VisiblePosition::VisiblePosition(const dom::Position &position, Affinity affinity) noexcept
      : _deepPosition {CanonicalPosition(position)}
  {
    if (affinity == Affinity::Upstream && !IsNull())
    {
      auto upstreamCopy = *this;
      upstreamCopy._affinity = Affinity::Upstream;
      if (!InSameLine(*this, upstreamCopy))
      {
        _affinity = Affinity::Upstream;
      }
    }
  }

  VisiblePosition VisiblePosition::Next(EditingBoundaryCrossingRule rule,
                                        bool *reachedBoundary) const noexcept
  {
    if (reachedBoundary)
    {
      *reachedBoundary = false;
    }
    // FIXME: Support CanSkipEditingBoundary
    krys_debug_assert(rule == EditingBoundaryCrossingRule::CanCrossEditingBoundary
                      || rule == EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
    VisiblePosition next(NextVisuallyDistinctCandidate(_deepPosition), _affinity);

    if (rule == EditingBoundaryCrossingRule::CanCrossEditingBoundary)
    {
      return next;
    }

    return HonorEditingBoundaryAtOrAfter(next, reachedBoundary);
  }

  VisiblePosition VisiblePosition::Previous(EditingBoundaryCrossingRule rule,
                                            bool *reachedBoundary) const noexcept
  {
    if (reachedBoundary)
    {
      *reachedBoundary = false;
    }
    // FIXME: Support CanSkipEditingBoundary
    krys_debug_assert(rule == EditingBoundaryCrossingRule::CanCrossEditingBoundary
                      || rule == EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
    // find first previous DOM position that is visible
    dom::Position pos = PreviousVisuallyDistinctCandidate(_deepPosition);

    // return null visible position if there is no previous visible position
    if (pos.AtStartOfTree())
    {
      if (reachedBoundary)
      {
        *reachedBoundary = true;
      }
      return VisiblePosition();
    }

    VisiblePosition prev = pos;
    krys_debug_assert(prev != *this);

    if (rule == EditingBoundaryCrossingRule::CanCrossEditingBoundary)
    {
      return prev;
    }

    return HonorEditingBoundaryAtOrBefore(prev, reachedBoundary);
  }

  dom::Position VisiblePosition::LeftVisuallyDistinctCandidate() const noexcept
  {
    dom::Position p = _deepPosition;
    if (p.IsNull())
    {
      return dom::Position();
    }

    dom::Position downstreamStart = p.Downstream();
    TextDirection primaryDirection = p.PrimaryDirection();

    inline_iterator::LineLogicalOrderCache orderCache;

    while (true)
    {
      auto [box, offset] = p.GetInlineBoxAndOffset(_affinity, primaryDirection);
      if (!box)
      {
        return primaryDirection == TextDirection::LTR ? PreviousVisuallyDistinctCandidate(_deepPosition)
                                                      : NextVisuallyDistinctCandidate(_deepPosition);
      }

      const RenderObject *renderer = &box->Renderer();

      while (true)
      {
        if ((renderer->IsBlockLevelReplacedOrAtomicInline() || renderer->IsBR())
            && offset == box->RightmostCaretOffset())
        {
          return box->IsLeftToRightDirection() ? PreviousVisuallyDistinctCandidate(_deepPosition)
                                               : NextVisuallyDistinctCandidate(_deepPosition);
        }

        if (!renderer->GetNode())
        {
          box.TraverseLineLeftwardOnLine();
          if (!box)
          {
            return primaryDirection == TextDirection::LTR ? PreviousVisuallyDistinctCandidate(_deepPosition)
                                                          : NextVisuallyDistinctCandidate(_deepPosition);
          }
          renderer = &box->Renderer();
          offset = box->RightmostCaretOffset();
          continue;
        }

        // Note that this may underflow the (unsigned) offset. This is fine and handled below.
        offset = static_cast<uint32>(box->IsLeftToRightDirection()
                                       ? renderer->PreviousOffset(static_cast<int32>(offset))
                                       : renderer->NextOffset(static_cast<int32>(offset)));

        auto caretMinOffset = box->MinimumCaretOffset();
        auto caretMaxOffset = box->MaximumCaretOffset();

        if (offset > caretMinOffset && offset < caretMaxOffset)
        {
          break;
        }

        if (offset != box->LeftmostCaretOffset())
        {
          // Overshot to the left.
          auto previousBox = box->NextLineLeftwardOnLineIgnoringLineBreak();
          if (!previousBox)
          {
            dom::Position positionOnLeft = primaryDirection == TextDirection::LTR
                                             ? PreviousVisuallyDistinctCandidate(_deepPosition)
                                             : NextVisuallyDistinctCandidate(_deepPosition);
            auto boxOnLeft = positionOnLeft.GetInlineBoxAndOffset(_affinity, primaryDirection).Box;
            if (boxOnLeft && boxOnLeft->GetLineBox() == box->GetLineBox())
            {
              return dom::Position();
            }
            return positionOnLeft;
          }

          // Reposition at the other logical position corresponding to our edge's visual position and go for
          // another round.
          box = previousBox;
          renderer = &box->Renderer();
          offset = previousBox->RightmostCaretOffset();
          continue;
        }

        uint8 level = box->BidiLevel();
        auto previousBox = box->NextLineLeftwardOnLine();

        if (box->Direction() == primaryDirection)
        {
          if (!previousBox)
          {
            auto logicalStart =
              primaryDirection == TextDirection::LTR
                ? inline_iterator::FirstLeafOnLineInLogicalOrderWithNode(box->GetLineBox(), orderCache)
                : inline_iterator::LastLeafOnLineInLogicalOrderWithNode(box->GetLineBox(), orderCache);
            if (logicalStart)
            {
              box = logicalStart;
              renderer = &box->Renderer();
              offset = primaryDirection == TextDirection::LTR ? box->MinimumCaretOffset()
                                                              : box->MaximumCaretOffset();
            }
            break;
          }
          if (previousBox->BidiLevel() >= level)
          {
            break;
          }

          level = previousBox->BidiLevel();

          auto nextBox = box;
          do
          {
            nextBox.TraverseLineRightwardOnLine();
          } while (nextBox && nextBox->BidiLevel() > level);

          if (nextBox && nextBox->BidiLevel() == level)
          {
            break;
          }

          box = previousBox;
          renderer = &box->Renderer();
          offset = box->RightmostCaretOffset();
          if (box->Direction() == primaryDirection)
          {
            break;
          }
          continue;
        }

        while (previousBox && !previousBox->Renderer().GetNode())
        {
          previousBox.TraverseLineLeftwardOnLine();
        }

        if (previousBox)
        {
          box = previousBox;
          renderer = &box->Renderer();
          offset = box->RightmostCaretOffset();
          if (box->BidiLevel() > level)
          {
            do
            {
              previousBox = previousBox.TraverseLineLeftwardOnLine();
            } while (previousBox && previousBox->BidiLevel() > level);

            if (!previousBox || previousBox->BidiLevel() < level)
            {
              continue;
            }
          }
        }
        else
        {
          // Trailing edge of a secondary box. Set to the leading edge of the entire box.
          while (true)
          {
            while (auto nextBox = box->NextLineRightwardOnLine())
            {
              if (nextBox->BidiLevel() < level)
              {
                break;
              }
              box = nextBox;
            }
            if (box->BidiLevel() == level)
            {
              break;
            }
            level = box->BidiLevel();
            while (auto prevBox = box->NextLineLeftwardOnLine())
            {
              if (prevBox->BidiLevel() < level)
              {
                break;
              }
              box = prevBox;
            }
            if (box->BidiLevel() == level)
            {
              break;
            }
            level = box->BidiLevel();
          }
          renderer = &box->Renderer();
          offset =
            primaryDirection == TextDirection::LTR ? box->MinimumCaretOffset() : box->MaximumCaretOffset();
        }
        break;
      }

      p = dom::MakeDeprecatedLegacyPosition(ShareRefPtr(renderer->GetNode()), offset);

      if ((p.IsCandidate() && p.Downstream() != downstreamStart) || p.AtStartOfTree() || p.AtEndOfTree())
      {
        return p;
      }

      krys_debug_assert(p != _deepPosition);
    }
  }

  VisiblePosition VisiblePosition::Left(bool stayInEditableContent, bool *reachedBoundary) const noexcept
  {
    if (reachedBoundary)
    {
      *reachedBoundary = false;
    }
    dom::Position pos = LeftVisuallyDistinctCandidate();
    // FIXME: Why can't we move left from the last position in a tree?
    if (pos.AtStartOfTree() || pos.AtEndOfTree())
    {
      if (reachedBoundary)
      {
        *reachedBoundary = true;
      }
      return VisiblePosition();
    }

    VisiblePosition left = pos;
    krys_debug_assert(left != *this);

    if (!stayInEditableContent)
    {
      return left;
    }

    return DirectionOfEnclosingBlock(left.DeepEquivalent()) == TextDirection::LTR
             ? HonorEditingBoundaryAtOrBefore(left, reachedBoundary)
             : HonorEditingBoundaryAtOrAfter(left, reachedBoundary);
  }

  dom::Position VisiblePosition::RightVisuallyDistinctCandidate() const noexcept
  {
    dom::Position p = _deepPosition;
    if (p.IsNull())
    {
      return dom::Position();
    }

    dom::Position downstreamStart = p.Downstream();
    TextDirection primaryDirection = p.PrimaryDirection();

    inline_iterator::LineLogicalOrderCache orderCache;

    while (true)
    {
      auto [box, offset] = p.GetInlineBoxAndOffset(_affinity, primaryDirection);
      if (!box)
      {
        return primaryDirection == TextDirection::LTR ? NextVisuallyDistinctCandidate(_deepPosition)
                                                      : PreviousVisuallyDistinctCandidate(_deepPosition);
      }

      const RenderObject *renderer = &box->Renderer();

      while (true)
      {
        if ((renderer->IsBlockLevelReplacedOrAtomicInline() || renderer->IsBR())
            && offset == box->LeftmostCaretOffset())
        {
          return box->IsLeftToRightDirection() ? NextVisuallyDistinctCandidate(_deepPosition)
                                               : PreviousVisuallyDistinctCandidate(_deepPosition);
        }

        if (!renderer->GetNode())
        {
          box.TraverseLineRightwardOnLine();
          if (!box)
          {
            return primaryDirection == TextDirection::LTR ? NextVisuallyDistinctCandidate(_deepPosition)
                                                          : PreviousVisuallyDistinctCandidate(_deepPosition);
          }
          renderer = &box->Renderer();
          offset = box->LeftmostCaretOffset();
          continue;
        }

        // Note that this may underflow the (unsigned) offset. This is fine and handled below.
        offset = static_cast<uint32>(box->IsLeftToRightDirection()
                                       ? renderer->NextOffset(static_cast<int32>(offset))
                                       : renderer->PreviousOffset(static_cast<int32>(offset)));

        auto caretMinOffset = box->MinimumCaretOffset();
        auto caretMaxOffset = box->MaximumCaretOffset();

        if (offset > caretMinOffset && offset < caretMaxOffset)
        {
          break;
        }

        if (offset != box->RightmostCaretOffset())
        {
          // Overshot to the right.
          auto nextBox = box->NextLineRightwardOnLineIgnoringLineBreak();
          if (!nextBox)
          {
            dom::Position positionOnRight = primaryDirection == TextDirection::LTR
                                              ? NextVisuallyDistinctCandidate(_deepPosition)
                                              : PreviousVisuallyDistinctCandidate(_deepPosition);
            auto boxOnRight = positionOnRight.GetInlineBoxAndOffset(_affinity, primaryDirection).Box;
            if (boxOnRight && boxOnRight->GetLineBox() == box->GetLineBox())
            {
              return dom::Position();
            }
            return positionOnRight;
          }

          // Reposition at the other logical position corresponding to our edge's visual position and go for
          // another round.
          box = nextBox;
          renderer = &box->Renderer();
          offset = nextBox->LeftmostCaretOffset();
          continue;
        }

        uint8 level = box->BidiLevel();
        auto nextBox = box->NextLineRightwardOnLine();

        if (box->Direction() == primaryDirection)
        {
          if (!nextBox)
          {
            auto logicalEnd =
              primaryDirection == TextDirection::LTR
                ? inline_iterator::LastLeafOnLineInLogicalOrderWithNode(box->GetLineBox(), orderCache)
                : inline_iterator::FirstLeafOnLineInLogicalOrderWithNode(box->GetLineBox(), orderCache);

            if (logicalEnd)
            {
              box = logicalEnd;
              renderer = &box->Renderer();
              offset = primaryDirection == TextDirection::LTR ? box->MaximumCaretOffset()
                                                              : box->MinimumCaretOffset();
            }
            break;
          }

          if (nextBox->BidiLevel() >= level)
          {
            break;
          }

          level = nextBox->BidiLevel();

          auto previousBox = box;
          do
          {
            previousBox.TraverseLineLeftwardOnLine();
          } while (previousBox && previousBox->BidiLevel() > level);

          if (previousBox && previousBox->BidiLevel() == level) // For example, abc FED 123 ^ CBA
          {
            break;
          }

          // For example, abc 123 ^ CBA or 123 ^ CBA abc
          box = nextBox;
          renderer = &box->Renderer();
          offset = box->LeftmostCaretOffset();
          if (box->Direction() == primaryDirection)
          {
            break;
          }
          continue;
        }

        while (nextBox && !nextBox->Renderer().GetNode())
        {
          nextBox.TraverseLineRightwardOnLine();
        }

        if (nextBox)
        {
          box = nextBox;
          renderer = &box->Renderer();
          offset = box->LeftmostCaretOffset();

          if (box->BidiLevel() > level)
          {
            do
            {
              nextBox.TraverseLineRightwardOnLine();
            } while (nextBox && nextBox->BidiLevel() > level);

            if (!nextBox || nextBox->BidiLevel() < level)
            {
              continue;
            }
          }
        }
        else
        {
          // Trailing edge of a secondary box. Set to the leading edge of the entire box.
          while (true)
          {
            while (auto previousBox = box->NextLineLeftwardOnLine())
            {
              if (previousBox->BidiLevel() < level)
              {
                break;
              }
              box = previousBox;
            }
            if (box->BidiLevel() == level)
            {
              break;
            }
            level = box->BidiLevel();
            while (auto followingBox = box->NextLineRightwardOnLine())
            {
              if (followingBox->BidiLevel() < level)
              {
                break;
              }
              box = followingBox;
            }
            if (box->BidiLevel() == level)
            {
              break;
            }
            level = box->BidiLevel();
          }
          renderer = &box->Renderer();
          offset =
            primaryDirection == TextDirection::LTR ? box->MaximumCaretOffset() : box->MinimumCaretOffset();
        }
        break;
      }

      p = dom::MakeDeprecatedLegacyPosition(ShareRefPtr(renderer->GetNode()), offset);

      if ((p.IsCandidate() && p.Downstream() != downstreamStart) || p.AtStartOfTree() || p.AtEndOfTree())
      {
        return p;
      }

      krys_debug_assert(p != _deepPosition);
    }
  }

  VisiblePosition VisiblePosition::Right(bool stayInEditableContent, bool *reachedBoundary) const noexcept
  {
    if (reachedBoundary)
    {
      *reachedBoundary = false;
    }
    dom::Position pos = RightVisuallyDistinctCandidate();
    // FIXME: Why can't we move left from the last position in a tree?
    if (pos.AtStartOfTree() || pos.AtEndOfTree())
    {
      if (reachedBoundary)
      {
        *reachedBoundary = true;
      }
      return VisiblePosition();
    }

    VisiblePosition right = pos;
    krys_debug_assert(right != *this);

    if (!stayInEditableContent)
    {
      return right;
    }

    return DirectionOfEnclosingBlock(right.DeepEquivalent()) == TextDirection::LTR
             ? HonorEditingBoundaryAtOrAfter(right, reachedBoundary)
             : HonorEditingBoundaryAtOrBefore(right, reachedBoundary);
  }

  VisiblePosition VisiblePosition::HonorEditingBoundaryAtOrBefore(const VisiblePosition &position,
                                                                  bool *reachedBoundary) const noexcept
  {
    if (reachedBoundary)
    {
      *reachedBoundary = false;
    }
    if (position.IsNull())
    {
      return position;
    }

    auto highestRoot = HighestEditableRoot(DeepEquivalent());

    // Return empty position if pos is not somewhere inside the editable region containing this position
    if (highestRoot && !position.DeepEquivalent().DeprecatedNode()->IsDescendantOf(*highestRoot))
    {
      if (reachedBoundary)
      {
        *reachedBoundary = true;
      }
      return VisiblePosition();
    }

    // Return position itself if the two are from the very same editable region, or both are non-editable
    // FIXME: In the non-editable case, just because the new position is non-editable doesn't mean movement
    // to it is allowed.  VisibleSelection::adjustForEditableContent has this problem too.
    if (HighestEditableRoot(position.DeepEquivalent()) == highestRoot)
    {
      if (reachedBoundary)
      {
        *reachedBoundary = *this == position;
      }
      return position;
    }

    // Return empty position if this position is non-editable, but pos is editable
    // FIXME: Move to the previous non-editable region.
    if (!highestRoot)
    {
      if (reachedBoundary)
      {
        *reachedBoundary = true;
      }
      return VisiblePosition();
    }

    // Return the last position before pos that is in the same editable region as this position
    return LastEditablePositionBeforePositionInRoot(position.DeepEquivalent(), highestRoot.get());
  }

  VisiblePosition VisiblePosition::HonorEditingBoundaryAtOrAfter(const VisiblePosition &otherPosition,
                                                                 bool *reachedBoundary) const noexcept
  {
    if (reachedBoundary)
    {
      *reachedBoundary = false;
    }
    if (otherPosition.IsNull())
    {
      return otherPosition;
    }

    auto highestRoot = HighestEditableRoot(DeepEquivalent());

    // Return empty position if otherPosition is not somewhere inside the editable region containing this
    // position
    if (highestRoot && !otherPosition.DeepEquivalent().DeprecatedNode()->IsDescendantOf(*highestRoot))
    {
      if (reachedBoundary)
      {
        *reachedBoundary = true;
      }
      return VisiblePosition();
    }

    // Return otherPosition itself if the two are from the very same editable region, or both are non-editable
    // FIXME: In the non-editable case, just because the new position is non-editable doesn't mean movement
    // to it is allowed.  VisibleSelection::adjustForEditableContent has this problem too.
    if (HighestEditableRoot(otherPosition.DeepEquivalent()) == highestRoot)
    {
      if (reachedBoundary)
      {
        *reachedBoundary = *this == otherPosition;
      }
      return otherPosition;
    }

    // Return empty position if this position is non-editable, but otherPosition is editable
    // FIXME: Move to the next non-editable region.
    if (!highestRoot)
    {
      if (reachedBoundary)
      {
        *reachedBoundary = true;
      }
      return VisiblePosition();
    }

    // Return the next position after pos that is in the same editable region as this position
    return FirstEditablePositionAfterPositionInRoot(otherPosition.DeepEquivalent(), highestRoot.get());
  }

  namespace
  {
    KRYS_NODISCARD dom::Position CanonicalizeCandidate(const dom::Position &candidate) noexcept
    {
      if (candidate.IsNull())
      {
        return dom::Position();
      }
      krys_debug_assert(candidate.IsCandidate());
      dom::Position upstream = candidate.Upstream();
      if (upstream.IsCandidate())
      {
        return upstream;
      }
      return candidate;
    }
  }

  dom::Position VisiblePosition::CanonicalPosition(const dom::Position &passedPosition) noexcept
  {
    // The updateLayout call below can do so much that even the position passed
    // in to us might get changed as a side effect. Specifically, there are code
    // paths that pass selection endpoints, and updateLayout can change the selection.
    dom::Position position = passedPosition;

    // FIXME (9535):  Canonicalizing to the leftmost candidate means that if we're at a line wrap, we will
    // ask renderers to paint downstream carets for other renderers.
    // To fix this, we need to either a) add code to all paintCarets to pass the responsibility off to
    // the appropriate renderer for VisiblePosition's like these, or b) canonicalize to the rightmost
    // candidate unless the affinity is upstream.
    if (position.IsNull())
    {
      return dom::Position();
    }

    krys_debug_assert(position.GetDocument());
    position.GetDocument()->UpdateLayoutIgnorePendingStylesheets();

    auto *node = position.GetContainerNode();

    dom::Position candidate = position.Upstream();
    if (candidate.IsCandidate())
    {
      return candidate;
    }
    candidate = position.Downstream();
    if (candidate.IsCandidate())
    {
      return candidate;
    }

    // When neither upstream or downstream gets us to a candidate (upstream/downstream won't leave
    // blocks or enter new ones), we search forward and backward until we find one.
    dom::Position next = CanonicalizeCandidate(NextCandidate(position));
    dom::Position prev = CanonicalizeCandidate(PreviousCandidate(position));
    auto *nextNode = next.DeprecatedNode();
    auto *prevNode = prev.DeprecatedNode();

    // The new position must be in the same editable element. Enforce that first.
    // Unless the descent is from a non-editable html element to an editable body.
    if (node && Is<html::HTMLHtmlElement>(*node) && !node->HasEditableStyle())
    {
      auto body = node->NodeDocument().Body();
      if (body && body->HasEditableStyle())
      {
        return next.IsNotNull() ? next : prev;
      }
    }

    auto *editingRoot = EditableRootForPosition(position);

    // If the html element is editable, descending into its body will look like a descent
    // from non-editable to editable content since rootEditableElement() always stops at the body.
    if ((editingRoot && Is<html::HTMLHtmlElement>(*editingRoot))
        || (node && (Is<dom::Document>(*node) || Is<dom::ShadowRoot>(*node))))
    {
      return next.IsNotNull() ? next : prev;
    }

    bool prevIsInSameEditableElement = prevNode && EditableRootForPosition(prev) == editingRoot;
    bool nextIsInSameEditableElement = nextNode && EditableRootForPosition(next) == editingRoot;
    if (prevIsInSameEditableElement && !nextIsInSameEditableElement)
    {
      return prev;
    }

    if (nextIsInSameEditableElement && !prevIsInSameEditableElement)
    {
      return next;
    }

    if (!nextIsInSameEditableElement && !prevIsInSameEditableElement)
    {
      return dom::Position();
    }

    // The new position should be in the same block flow element. Favor that.
    auto *originalBlock = DeprecatedEnclosingBlockFlowElement(node);
    bool nextIsOutsideOriginalBlock = !nextNode->IsDescendantOf(originalBlock) && nextNode != originalBlock;
    bool prevIsOutsideOriginalBlock = !prevNode->IsDescendantOf(originalBlock) && prevNode != originalBlock;
    if (nextIsOutsideOriginalBlock && !prevIsOutsideOriginalBlock)
    {
      return prev;
    }

    return next;
  }

  char32 VisiblePosition::CharacterAfter() const noexcept
  {
    // We canonicalize to the first of two equivalent candidates, but the second of the two candidates
    // is the one that will be inside the text node containing the character after this visible position.
    dom::Position pos = _deepPosition.Downstream();
    if (!pos.GetContainerNode() || !Is<dom::Text>(*pos.GetContainerNode()))
    {
      return 0;
    }
    switch (pos.GetAnchorType())
    {
      case dom::Position::AnchorType::PositionIsAfterChildren:
      case dom::Position::AnchorType::PositionIsAfterAnchor:
      case dom::Position::AnchorType::PositionIsBeforeAnchor:
      case dom::Position::AnchorType::PositionIsBeforeChildren: return 0;
      case dom::Position::AnchorType::PositionIsOffsetInAnchor: break;
    }
    int32 offset = pos.OffsetInContainerNode();
    auto *textNode = pos.ContainerText();
    const auto &data = textNode->Data();
    int32 length = static_cast<int32>(data.size());
    if (offset >= length)
    {
      return 0;
    }

    UChar32 ch;
    U8_NEXT(reinterpret_cast<const uint8_t *>(data.data()), offset, length, ch);
    return ch < 0 ? 0 : static_cast<char32>(ch);
  }

  InlineBoxAndOffset VisiblePosition::GetInlineBoxAndOffset() const noexcept
  {
    return _deepPosition.GetInlineBoxAndOffset(_affinity);
  }

  InlineBoxAndOffset VisiblePosition::GetInlineBoxAndOffset(TextDirection primaryDirection) const noexcept
  {
    return _deepPosition.GetInlineBoxAndOffset(_affinity, primaryDirection);
  }

  VisiblePosition::LocalCaretRectResult VisiblePosition::LocalCaretRect() const noexcept
  {
    auto *node = _deepPosition.AnchorNode();
    if (!node)
    {
      return {};
    }

    auto boxAndOffset = GetInlineBoxAndOffset();
    auto *renderer =
      boxAndOffset.Box ? const_cast<RenderObject *>(&boxAndOffset.Box->Renderer()) : node->Renderer();
    if (!renderer)
    {
      return {};
    }

    return {ComputeLocalCaretRect(*renderer, boxAndOffset), renderer};
  }

  IntRect VisiblePosition::AbsoluteCaretBounds(bool *insideFixed) const noexcept
  {
    RenderBlock *renderer = nullptr;
    LayoutRect localRect = LocalCaretRectInRendererForCaretPainting(*this, renderer);
    return AbsoluteBoundsForLocalCaretRect(renderer, localRect, insideFixed);
  }

  FloatRect VisiblePosition::AbsoluteSelectionBoundsForLine() const noexcept
  {
    auto box = GetInlineBoxAndOffset().Box;
    if (!box)
    {
      return {};
    }

    auto line = box->GetLineBox();
    return line->FormattingContextRoot()
      .LocalToAbsoluteQuad(FloatQuad(LineSelection::PhysicalRect(*line)))
      .BoundingBox();
  }

  int32 VisiblePosition::LineDirectionPointForBlockDirectionNavigation() const noexcept
  {
    auto localRect = LocalCaretRect();
    if (localRect.Rect.IsEmpty() || !localRect.Renderer)
    {
      return 0;
    }

    // This ignores transforms on purpose, for now. Vertical navigation is done
    // without consulting transforms, so that 'up' in transformed text is 'up'
    // relative to the text, not absolute 'up'.
    auto *renderer = localRect.Renderer;
    auto caretPoint = renderer->LocalToAbsolute(FloatPoint(localRect.Rect.Location()));
    const RenderObject *containingBlock = renderer->ContainingBlock();
    if (!containingBlock)
    {
      containingBlock =
        renderer; // Just use ourselves to determine the writing mode if we have no containing block.
    }
    return static_cast<int32>(containingBlock->IsHorizontalWritingMode() ? caretPoint.X() : caretPoint.Y());
  }

  // FIXME: Maybe this should be deprecated too, like the underlying function?
  dom::Element *EnclosingBlockFlowElement(const VisiblePosition &visiblePosition) noexcept
  {
    if (visiblePosition.IsNull())
    {
      return nullptr;
    }

    return DeprecatedEnclosingBlockFlowElement(visiblePosition.DeepEquivalent().DeprecatedNode());
  }

  bool IsFirstVisiblePositionInNode(const VisiblePosition &visiblePosition, const dom::Node *node) noexcept
  {
    if (visiblePosition.IsNull())
    {
      return false;
    }

    if (!visiblePosition.DeepEquivalent().GetContainerNode()->IsDescendantOf(node))
    {
      return false;
    }

    VisiblePosition previous = visiblePosition.Previous();
    return previous.IsNull() || !previous.DeepEquivalent().DeprecatedNode()->IsDescendantOf(node);
  }

  bool IsLastVisiblePositionInNode(const VisiblePosition &visiblePosition, const dom::Node *node) noexcept
  {
    if (visiblePosition.IsNull())
    {
      return false;
    }

    if (!visiblePosition.DeepEquivalent().GetContainerNode()->IsDescendantOf(node))
    {
      return false;
    }

    VisiblePosition next = visiblePosition.Next();
    return next.IsNull() || !next.DeepEquivalent().DeprecatedNode()->IsDescendantOf(node);
  }

  bool AreVisiblePositionsInSameTreeScope(const VisiblePosition &a, const VisiblePosition &b) noexcept
  {
    auto *nodeA = a.DeepEquivalent().AnchorNode();
    auto *nodeB = b.DeepEquivalent().AnchorNode();
    return nodeA && nodeB && nodeA->IsConnected() && nodeB->IsConnected()
           && &dom::TreeQueries::Root(*nodeA) == &dom::TreeQueries::Root(*nodeB);
  }

  bool VisiblePosition::Equals(const VisiblePosition &other) const noexcept
  {
    return _affinity == other._affinity && _deepPosition.Equals(other._deepPosition);
  }

  Maybe<dom::BoundaryPoint> MakeBoundaryPoint(const VisiblePosition &position) noexcept
  {
    return dom::MakeBoundaryPoint(position.DeepEquivalent());
  }

  dom::Node *CommonInclusiveAncestor(const VisiblePosition &a, const VisiblePosition &b) noexcept
  {
    return dom::CommonInclusiveAncestor(a.DeepEquivalent(), b.DeepEquivalent());
  }

  Maybe<dom::SimpleRange> MakeSimpleRange(const VisiblePositionRange &range) noexcept
  {
    return dom::MakeSimpleRange(range.Start, range.End);
  }

  VisiblePositionRange MakeVisiblePositionRange(const Maybe<dom::SimpleRange> &range) noexcept
  {
    if (!range)
    {
      return {};
    }
    return {dom::MakeContainerOffsetPosition(range->Start), dom::MakeContainerOffsetPosition(range->End)};
  }

  std::partial_ordering operator<=>(const VisiblePosition &a, const VisiblePosition &b) noexcept
  {
    // FIXME: Should two positions with different affinity be considered equivalent or not?
    return dom::TreeOrder<dom::TreeType::ComposedTree>(a.DeepEquivalent(), b.DeepEquivalent());
  }

  bool Intersects(const VisiblePositionRange &a, const VisiblePositionRange &b) noexcept
  {
    return a.Start <= b.End && b.Start <= a.End;
  }

  bool Contains(const VisiblePositionRange &range, const VisiblePosition &point) noexcept
  {
    return point >= range.Start && point <= range.End;
  }

  VisiblePositionRange Intersection(const VisiblePositionRange &a, const VisiblePositionRange &b) noexcept
  {
    return {std::max(a.Start, b.Start), std::min(a.End, b.End)};
  }

  dom::Node *CommonInclusiveAncestor(const VisiblePositionRange &range) noexcept
  {
    return CommonInclusiveAncestor(range.Start, range.End);
  }

  VisiblePosition Midpoint(const VisiblePositionRange &range) noexcept
  {
    auto *rootNode = CommonInclusiveAncestor(range);
    if (!rootNode)
    {
      return {};
    }
    auto *rootContainerNode = DynamicDowncast<dom::ContainerNode>(rootNode);
    if (!rootContainerNode)
    {
      rootContainerNode = rootNode->ParentNode();
    }
    if (!rootContainerNode)
    {
      return {};
    }
    auto scope = dom::MakeRangeSelectingNodeContents(*rootContainerNode);
    auto characterRange = GetCharacterRange(scope, *dom::MakeSimpleRange(range.Start, range.End));
    return dom::MakeContainerOffsetPosition(
      ResolveCharacterLocation(scope, characterRange.Location + characterRange.Length / 2));
  }
}

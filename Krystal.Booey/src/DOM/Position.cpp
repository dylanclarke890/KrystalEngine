#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/PositionIterator.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/InlineRunAndOffset.hpp"
#include "Krystal.Booey/Editing/VisiblePosition.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLBRElement.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/HTMLTableElement.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLogicalOrderTraversal.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGTextElement.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <climits>
#include <unicode/utf8.h>

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD bool HasInlineRun(const RenderObject &renderer) noexcept
    {
      if (auto *renderBox = DynamicDowncast<RenderBox>(renderer);
          renderBox && inline_iterator::BoxFor(*renderBox))
      {
        return true;
      }
      if (auto *renderText = DynamicDowncast<RenderText>(renderer);
          renderText && inline_iterator::LineLeftmostTextBoxFor(*renderText))
      {
        return true;
      }
      if (auto *renderLineBreak = DynamicDowncast<RenderLineBreak>(renderer);
          renderLineBreak && inline_iterator::BoxFor(*renderLineBreak))
      {
        return true;
      }
      return false;
    }

    KRYS_NODISCARD Node *NextRenderedEditable(Node *node) noexcept
    {
      while ((node = NextLeafNode(node)))
      {
        auto *renderer = node->Renderer();
        if (!renderer || !node->HasEditableStyle())
        {
          continue;
        }
        if (HasInlineRun(*renderer))
        {
          return node;
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD Node *PreviousRenderedEditable(Node *node) noexcept
    {
      while ((node = PreviousLeafNode(node)))
      {
        auto *renderer = node->Renderer();
        if (!renderer || !node->HasEditableStyle())
        {
          continue;
        }
        if (HasInlineRun(*renderer))
        {
          return node;
        }
      }
      return nullptr;
    }

    /// @brief The code point at @p offset in @p node's data, or 0 if it's past the end.
    KRYS_NODISCARD char32 CodePointAt(const Text &text, uint32 offset) noexcept
    {
      const auto &data = text.Data();
      if (offset >= data.size())
      {
        return 0;
      }
      int32 index = static_cast<int32>(offset);
      UChar32 codePoint;
      U8_NEXT(reinterpret_cast<const uint8_t *>(data.data()), index, static_cast<int32>(data.size()),
              codePoint);
      return codePoint < 0 ? 0 : static_cast<char32>(codePoint);
    }
  }

  Position::Position(RefPtr<Node> &&anchorNode, uint32 offset, LegacyEditingPositionFlag) noexcept
      : _anchorNode(krys::move(anchorNode)), _offset(offset),
        _anchorType(AnchorTypeForLegacyEditingPosition(_anchorNode.get(), _offset)),
        _isLegacyEditingPosition(true)
  {
    krys_debug_assert(!_anchorNode || !Is<ShadowRoot>(*_anchorNode)
                      || _anchorNode.get() == GetContainerNode());
    krys_debug_assert(!_anchorNode || !Is<PseudoElement>(*_anchorNode));
  }

  Position::Position(RefPtr<Node> &&anchorNode, AnchorType anchorType) noexcept
      : _anchorNode(krys::move(anchorNode)), _offset(0), _anchorType(anchorType),
        _isLegacyEditingPosition(false)
  {
    krys_debug_assert(!_anchorNode || !Is<ShadowRoot>(*_anchorNode)
                      || _anchorNode.get() == GetContainerNode());
    krys_debug_assert(!_anchorNode || !Is<PseudoElement>(*_anchorNode));
    krys_debug_assert(anchorType != AnchorType::PositionIsOffsetInAnchor);
    krys_debug_assert(!((anchorType == AnchorType::PositionIsBeforeChildren
                         || anchorType == AnchorType::PositionIsAfterChildren)
                        && (Is<Text>(*_anchorNode) || EditingIgnoresContent(*_anchorNode))));
  }

  Position::Position(RefPtr<Node> &&anchorNode, uint32 offset, AnchorType anchorType) noexcept
      : _anchorNode(krys::move(anchorNode)), _offset(offset), _anchorType(anchorType),
        _isLegacyEditingPosition(false)
  {
    krys_debug_assert(anchorType == AnchorType::PositionIsOffsetInAnchor);
  }

  Position::Position(RefPtr<Text> &&textNode, uint32 offset) noexcept
      : _anchorNode(krys::move(textNode)), _offset(offset), _anchorType(AnchorType::PositionIsOffsetInAnchor),
        _isLegacyEditingPosition(false)
  {
    krys_debug_assert(_anchorNode);
  }

  void Position::MoveToPosition(Node *node, uint32 offset) noexcept
  {
    krys_debug_assert(!EditingIgnoresContent(*node));
    krys_debug_assert(GetAnchorType() == AnchorType::PositionIsOffsetInAnchor || _isLegacyEditingPosition);
    _anchorNode = ShareRefPtr(node);
    _offset = offset;
    if (_isLegacyEditingPosition)
    {
      _anchorType = AnchorTypeForLegacyEditingPosition(_anchorNode.get(), _offset);
    }
  }

  void Position::MoveToOffset(uint32 offset) noexcept
  {
    krys_debug_assert(GetAnchorType() == AnchorType::PositionIsOffsetInAnchor || _isLegacyEditingPosition);
    _offset = offset;
    if (_isLegacyEditingPosition)
    {
      _anchorType = AnchorTypeForLegacyEditingPosition(_anchorNode.get(), _offset);
    }
  }

  Node *Position::GetContainerNode() const noexcept
  {
    if (!_anchorNode)
    {
      return nullptr;
    }

    switch (GetAnchorType())
    {
      case AnchorType::PositionIsBeforeChildren:
      case AnchorType::PositionIsAfterChildren:
      case AnchorType::PositionIsOffsetInAnchor: return _anchorNode.get();
      case AnchorType::PositionIsBeforeAnchor:
      case AnchorType::PositionIsAfterAnchor:    return _anchorNode->ParentNode();
    }
    krys_unreachable();
  }

  Text *Position::ContainerText() const noexcept
  {
    switch (GetAnchorType())
    {
      case AnchorType::PositionIsOffsetInAnchor: return DynamicDowncast<Text>(_anchorNode.get());
      case AnchorType::PositionIsBeforeAnchor:
      case AnchorType::PositionIsAfterAnchor:    return nullptr;
      case AnchorType::PositionIsBeforeChildren:
      case AnchorType::PositionIsAfterChildren:
        krys_debug_assert(!_anchorNode || !Is<Text>(*_anchorNode));
        return nullptr;
    }
    krys_unreachable();
  }

  Element *Position::ContainerOrParentElement() const noexcept
  {
    auto *container = GetContainerNode();
    if (!container)
    {
      return nullptr;
    }
    if (auto *element = DynamicDowncast<Element>(*container))
    {
      return element;
    }
    return container->ParentElement();
  }

  int32 Position::ComputeOffsetInContainerNode() const noexcept
  {
    if (!_anchorNode)
    {
      return 0;
    }

    switch (GetAnchorType())
    {
      case AnchorType::PositionIsBeforeChildren: return 0;
      case AnchorType::PositionIsAfterChildren:  return static_cast<int32>(_anchorNode->Length());
      case AnchorType::PositionIsOffsetInAnchor: return static_cast<int32>(_offset);
      case AnchorType::PositionIsBeforeAnchor:   return static_cast<int32>(_anchorNode->ComputeNodeIndex());
      case AnchorType::PositionIsAfterAnchor:    return static_cast<int32>(_anchorNode->ComputeNodeIndex() + 1);
    }
    krys_unreachable();
  }

  int32 Position::OffsetForPositionAfterAnchor() const noexcept
  {
    krys_debug_assert(_anchorType == AnchorType::PositionIsAfterAnchor
                      || _anchorType == AnchorType::PositionIsAfterChildren);
    krys_debug_assert(!_isLegacyEditingPosition);
    auto *anchorNode = AnchorNode();
    krys_debug_assert(anchorNode);
    return anchorNode ? LastOffsetForEditing(*anchorNode) : 0;
  }

  // Neighbor-anchored positions are invalid DOM positions, so they need to be
  // fixed up before handing them off to the Range object.
  Position Position::ParentAnchoredEquivalent() const noexcept
  {
    auto *anchorNode = AnchorNode();
    if (!anchorNode)
    {
      return {};
    }

    // FIXME: This should only be necessary for legacy positions, but is also needed for positions before and
    // after Tables
    if (!_offset
        && (_anchorType != AnchorType::PositionIsAfterAnchor
            && _anchorType != AnchorType::PositionIsAfterChildren))
    {
      if (anchorNode->ParentNode() && (EditingIgnoresContent(*anchorNode) || IsRenderedTable(anchorNode)))
      {
        return PositionInParentBeforeNode(anchorNode);
      }
      return Position(ShareRefPtr(anchorNode), 0, AnchorType::PositionIsOffsetInAnchor);
    }

    if (!Is<CharacterData>(*anchorNode)
        && (_anchorType == AnchorType::PositionIsAfterAnchor
            || _anchorType == AnchorType::PositionIsAfterChildren || _offset == anchorNode->CountChildNodes())
        && (EditingIgnoresContent(*anchorNode) || IsRenderedTable(anchorNode)) && GetContainerNode())
    {
      return PositionInParentAfterNode(anchorNode);
    }

    return {ShareRefPtr(GetContainerNode()), static_cast<uint32>(ComputeOffsetInContainerNode()),
            AnchorType::PositionIsOffsetInAnchor};
  }

  RefPtr<Node> Position::FirstNode() const noexcept
  {
    auto *container = GetContainerNode();
    if (!container)
    {
      return nullptr;
    }
    if (Is<CharacterData>(*container))
    {
      return ShareRefPtr(container);
    }
    if (auto *node = ComputeNodeAfterPosition())
    {
      return ShareRefPtr(node);
    }
    if (!ComputeOffsetInContainerNode())
    {
      return ShareRefPtr(container);
    }
    return ShareRefPtr(TreeQueries::NextSkippingChildren(*container));
  }

  Node *Position::ComputeNodeBeforePosition() const noexcept
  {
    if (!_anchorNode)
    {
      return nullptr;
    }

    switch (GetAnchorType())
    {
      case AnchorType::PositionIsBeforeChildren: return nullptr;
      case AnchorType::PositionIsAfterChildren:  return _anchorNode->LastChild();
      case AnchorType::PositionIsOffsetInAnchor:
        return _offset ? _anchorNode->TraverseToChildAt(_offset - 1) : nullptr;
      case AnchorType::PositionIsBeforeAnchor: return _anchorNode->PreviousSibling();
      case AnchorType::PositionIsAfterAnchor:  return _anchorNode.get();
    }
    krys_unreachable();
  }

  Node *Position::ComputeNodeAfterPosition() const noexcept
  {
    if (!_anchorNode)
    {
      return nullptr;
    }

    switch (GetAnchorType())
    {
      case AnchorType::PositionIsBeforeChildren: return _anchorNode->FirstChild();
      case AnchorType::PositionIsAfterChildren:  return nullptr;
      case AnchorType::PositionIsOffsetInAnchor: return _anchorNode->TraverseToChildAt(_offset);
      case AnchorType::PositionIsBeforeAnchor:   return _anchorNode.get();
      case AnchorType::PositionIsAfterAnchor:    return _anchorNode->NextSibling();
    }
    krys_unreachable();
  }

  Position::AnchorType Position::AnchorTypeForLegacyEditingPosition(Node *anchorNode, uint32 offset) noexcept
  {
    if (anchorNode && EditingIgnoresContent(*anchorNode))
    {
      if (offset == 0)
      {
        return AnchorType::PositionIsBeforeAnchor;
      }
      return AnchorType::PositionIsAfterAnchor;
    }
    return AnchorType::PositionIsOffsetInAnchor;
  }

  // FIXME: This method is confusing (does it return anchorNode() or containerNode()?) and should be renamed
  // or removed
  RefPtr<Element> Position::AnchorElementAncestor() const noexcept
  {
    for (auto *node = AnchorNode(); node; node = node->ParentNode())
    {
      if (auto *element = DynamicDowncast<Element>(*node))
      {
        return ShareRefPtr(element);
      }
    }
    return nullptr;
  }

  Position Position::Previous(PositionMoveType moveType) const noexcept
  {
    auto *node = DeprecatedNode();
    if (!node)
    {
      return *this;
    }

    // FIXME: Negative offsets shouldn't be allowed. We should catch this earlier.
    krys_debug_assert(DeprecatedEditingOffset() >= 0);

    uint32 offset = static_cast<uint32>(DeprecatedEditingOffset());

    if (GetAnchorType() == AnchorType::PositionIsBeforeAnchor)
    {
      node = GetContainerNode();
      if (!node)
      {
        return *this;
      }

      offset = static_cast<uint32>(ComputeOffsetInContainerNode());
    }

    if (offset > 0)
    {
      if (auto *child = node->TraverseToChildAt(offset - 1))
      {
        return LastPositionInOrAfterNode(child);
      }

      // There are two reasons child might be 0:
      //   1) The node is node like a text node that is not an element, and therefore has no children.
      //      Going backward one character at a time is correct.
      //   2) The old offset was a bogus offset like (<br>, 1), and there is no child.
      //      Going from 1 to 0 is correct.
      switch (moveType)
      {
        case PositionMoveType::CodePoint: return MakeDeprecatedLegacyPosition(ShareRefPtr(node), offset - 1);
        case PositionMoveType::Character:
        {
          auto previousOffset = UncheckedPreviousOffset(node, offset);
          return MakeDeprecatedLegacyPosition(ShareRefPtr(node), static_cast<uint32>(previousOffset));
        }
        case PositionMoveType::BackwardDeletion:
        {
          auto previousOffset = UncheckedPreviousOffsetForBackwardDeletion(node, offset);
          return MakeDeprecatedLegacyPosition(ShareRefPtr(node), static_cast<uint32>(previousOffset));
        }
      }
    }

    auto *parent = node->ParentNode();
    if (!parent)
    {
      return *this;
    }

    if (PositionBeforeOrAfterNodeIsCandidate(*node))
    {
      return PositionBeforeNode(node);
    }

    auto *previousSibling = node->PreviousSibling();
    if (previousSibling && PositionBeforeOrAfterNodeIsCandidate(*previousSibling))
    {
      return PositionAfterNode(previousSibling);
    }

    return MakeContainerOffsetPosition(ShareRefPtr<Node>(parent), node->ComputeNodeIndex());
  }

  Position Position::Next(PositionMoveType moveType) const noexcept
  {
    krys_debug_assert(moveType != PositionMoveType::BackwardDeletion);

    auto *node = DeprecatedNode();
    if (!node)
    {
      return *this;
    }

    // FIXME: Negative offsets shouldn't be allowed. We should catch this earlier.
    krys_debug_assert(DeprecatedEditingOffset() >= 0);

    uint32 offset = static_cast<uint32>(DeprecatedEditingOffset());

    if (GetAnchorType() == AnchorType::PositionIsAfterAnchor)
    {
      node = GetContainerNode();
      if (!node)
      {
        return *this;
      }

      offset = static_cast<uint32>(ComputeOffsetInContainerNode());
    }

    auto *child = node->TraverseToChildAt(offset);
    if (child || (!node->HasChildNodes() && offset < static_cast<uint32>(LastOffsetForEditing(*node))))
    {
      if (child)
      {
        return FirstPositionInOrBeforeNode(child);
      }

      // There are two reasons child might be 0:
      //   1) The node is node like a text node that is not an element, and therefore has no children.
      //      Going forward one character at a time is correct.
      //   2) The new offset is a bogus offset like (<br>, 1), and there is no child.
      //      Going from 0 to 1 is correct.
      auto nextOffset = moveType == PositionMoveType::Character
                          ? static_cast<uint32>(UncheckedNextOffset(node, offset))
                          : offset + 1;
      return MakeDeprecatedLegacyPosition(ShareRefPtr(node), nextOffset);
    }

    auto *parent = node->ParentNode();
    if (!parent)
    {
      return *this;
    }

    if (IsRenderedTable(node) || EditingIgnoresContent(*node))
    {
      return PositionAfterNode(node);
    }

    auto *nextSibling = node->NextSibling();
    if (nextSibling && PositionBeforeOrAfterNodeIsCandidate(*nextSibling))
    {
      return PositionBeforeNode(nextSibling);
    }

    return MakeContainerOffsetPosition(ShareRefPtr<Node>(parent), node->ComputeNodeIndex() + 1);
  }

  int32 Position::UncheckedPreviousOffset(const Node *node, uint32 current) noexcept
  {
    return node->Renderer() ? node->Renderer()->PreviousOffset(static_cast<int32>(current))
                            : static_cast<int32>(current) - 1;
  }

  int32 Position::UncheckedPreviousOffsetForBackwardDeletion(const Node *node, uint32 current) noexcept
  {
    return node->Renderer() ? node->Renderer()->PreviousOffsetForBackwardDeletion(static_cast<int32>(current))
                            : static_cast<int32>(current) - 1;
  }

  int32 Position::UncheckedNextOffset(const Node *node, uint32 current) noexcept
  {
    return node->Renderer() ? node->Renderer()->NextOffset(static_cast<int32>(current))
                            : static_cast<int32>(current) + 1;
  }

  bool Position::AtFirstEditingPositionForNode() const noexcept
  {
    if (IsNull())
    {
      return true;
    }
    // FIXME: Position before anchor shouldn't be considered as at the first editing position for node
    // since that position resides outside of the node.
    switch (_anchorType)
    {
      case AnchorType::PositionIsOffsetInAnchor: return !_offset;
      case AnchorType::PositionIsBeforeChildren:
      case AnchorType::PositionIsBeforeAnchor:   return true;
      case AnchorType::PositionIsAfterChildren:
      case AnchorType::PositionIsAfterAnchor:    return !LastOffsetForEditing(*DeprecatedNode());
    }
    krys_unreachable();
  }

  bool Position::AtLastEditingPositionForNode() const noexcept
  {
    if (IsNull())
    {
      return true;
    }
    // FIXME: Position after anchor shouldn't be considered as at the first editing position for node since
    // that position resides outside of the node.
    return _anchorType == AnchorType::PositionIsAfterAnchor
           || _anchorType == AnchorType::PositionIsAfterChildren
           || static_cast<int32>(_offset) >= LastOffsetForEditing(*DeprecatedNode());
  }

  // A position is considered at editing boundary if one of the following is true:
  // 1. It is the first position in the node and the next visually equivalent position
  //    is non editable.
  // 2. It is the last position in the node and the previous visually equivalent position
  //    is non editable.
  // 3. It is an editable position and both the next and previous visually equivalent
  //    positions are both non editable.
  bool Position::AtEditingBoundary() const noexcept
  {
    Position nextPosition = Downstream(EditingBoundaryCrossingRule::CanCrossEditingBoundary);
    if (AtFirstEditingPositionForNode() && nextPosition.IsNotNull()
        && !nextPosition.DeprecatedNode()->HasEditableStyle())
    {
      return true;
    }

    Position prevPosition = Upstream(EditingBoundaryCrossingRule::CanCrossEditingBoundary);
    if (AtLastEditingPositionForNode() && prevPosition.IsNotNull()
        && !prevPosition.DeprecatedNode()->HasEditableStyle())
    {
      return true;
    }

    return nextPosition.IsNotNull() && !nextPosition.DeprecatedNode()->HasEditableStyle()
           && prevPosition.IsNotNull() && !prevPosition.DeprecatedNode()->HasEditableStyle();
  }

  RefPtr<Node> Position::ParentEditingBoundary() const noexcept
  {
    if (!_anchorNode)
    {
      return nullptr;
    }

    auto documentElement = _anchorNode->NodeDocument().DocumentElement();
    if (!documentElement)
    {
      return nullptr;
    }

    Node *boundary = _anchorNode.get();
    while (boundary != documentElement.get() && boundary->NonShadowBoundaryParentNode()
           && _anchorNode->HasEditableStyle() == boundary->ParentNode()->HasEditableStyle())
    {
      boundary = boundary->NonShadowBoundaryParentNode();
    }

    return ShareRefPtr(boundary);
  }

  bool Position::AtStartOfTree() const noexcept
  {
    if (IsNull())
    {
      return true;
    }

    auto *container = GetContainerNode();
    if (container && container->ParentNode())
    {
      return false;
    }

    switch (_anchorType)
    {
      case AnchorType::PositionIsOffsetInAnchor: return !_offset;
      case AnchorType::PositionIsBeforeAnchor:   return !_anchorNode->PreviousSibling();
      case AnchorType::PositionIsAfterAnchor:    return false;
      case AnchorType::PositionIsBeforeChildren: return true;
      case AnchorType::PositionIsAfterChildren:  return !LastOffsetForEditing(*AnchorNode());
    }
    krys_unreachable();
  }

  bool Position::AtEndOfTree() const noexcept
  {
    if (IsNull())
    {
      return true;
    }

    auto *container = GetContainerNode();
    if (container && container->ParentNode())
    {
      return false;
    }

    switch (_anchorType)
    {
      case AnchorType::PositionIsOffsetInAnchor:
        return static_cast<int32>(_offset) >= LastOffsetForEditing(*AnchorNode());
      case AnchorType::PositionIsBeforeAnchor:   return false;
      case AnchorType::PositionIsAfterAnchor:    return !_anchorNode->NextSibling();
      case AnchorType::PositionIsBeforeChildren: return !LastOffsetForEditing(*AnchorNode());
      case AnchorType::PositionIsAfterChildren:  return true;
    }
    krys_unreachable();
  }

  // return first preceding DOM position rendered at a different location, or "this"
  Position Position::PreviousCharacterPosition(Affinity affinity) const noexcept
  {
    if (IsNull())
    {
      return {};
    }

    auto *fromRootEditableElement = DeprecatedNode()->RootEditableElement();

    bool atStartOfLine = IsStartOfLine(VisiblePosition(*this, affinity));
    bool rendered = IsCandidate();

    Position currentPosition = *this;
    while (!currentPosition.AtStartOfTree())
    {
      currentPosition = currentPosition.Previous();

      if (currentPosition.DeprecatedNode()->RootEditableElement() != fromRootEditableElement)
      {
        return *this;
      }

      if (atStartOfLine || !rendered)
      {
        if (currentPosition.IsCandidate())
        {
          return currentPosition;
        }
      }
      else if (RendersInDifferentPosition(currentPosition))
      {
        return currentPosition;
      }
    }

    return *this;
  }

  namespace
  {
    // Whether or not [node, 0] and [node, lastOffsetForEditing(node)] are their own VisiblePositions.
    // If true, adjacent candidates are visually distinct.
    // FIXME: Disregard nodes with renderers that have no height, as we do in isCandidate.
    // FIXME: Share code with isCandidate, if possible.
    KRYS_NODISCARD bool EndsOfNodeAreVisuallyDistinctPositions(Node *node) noexcept
    {
      if (!node || !node->Renderer())
      {
        return false;
      }

      if (!node->Renderer()->IsInline())
      {
        return true;
      }

      // Don't include inline tables.
      if (auto *element = DynamicDowncast<html::HTMLElement>(*node);
          element && Is<html::HTMLTableElement>(*element))
      {
        return false;
      }

      if (!node->Renderer()->IsBlockLevelReplacedOrAtomicInline() || !CanHaveChildrenForEditing(*node)
          || !Downcast<RenderBox>(*node->Renderer()).Height())
      {
        return false;
      }

      // There is a VisiblePosition inside an empty inline-block container.
      if (!node->HasChildNodes())
      {
        return true;
      }

      return !Position::HasRenderedNonAnonymousDescendantsWithHeight(
        Downcast<RenderElement>(*node->Renderer()));
    }

    KRYS_NODISCARD Node *EnclosingVisualBoundary(Node *node) noexcept
    {
      while (node && !EndsOfNodeAreVisuallyDistinctPositions(node))
      {
        node = node->ParentNode();
      }

      return node;
    }

    // upstream() and downstream() want to return positions that are either in a
    // text node or at just before a non-text node.  This method checks for that.
    KRYS_NODISCARD bool IsStreamer(const PositionIterator &position) noexcept
    {
      auto *node = position.GetNode();
      if (!node)
      {
        return true;
      }

      if (IsAtomicNode(node))
      {
        return true;
      }

      return position.AtStartOfNode();
    }

    KRYS_NODISCARD bool IsNonTextSVGElement(const Node &node) noexcept
    {
      auto *svgElement = DynamicDowncast<svg::SVGElement>(node);
      return svgElement && !Is<svg::SVGTextElement>(*svgElement);
    }
  }

  // This function and downstream() are used for moving back and forth between visually equivalent candidates.
  // For example, for the text node "foo     bar" where whitespace is collapsible, there are two candidates
  // that map to the VisiblePosition between 'b' and the space.  This function will return the left candidate
  // and downstream() will return the right one.
  // Also, upstream() will return [boundary, 0] for any of the positions from [boundary, 0] to the first
  // candidate in boundary, where endsOfNodeAreVisuallyDistinctPositions(boundary) is true.
  Position Position::Upstream(EditingBoundaryCrossingRule rule) const noexcept
  {
    auto *startNode = DeprecatedNode();
    if (!startNode)
    {
      return {};
    }

    // iterate backward from there, looking for a qualified position
    auto *boundary = EnclosingVisualBoundary(startNode);
    // FIXME: PositionIterator should respect Before and After positions.
    auto *anchorNode = _anchorNode.get();
    PositionIterator lastVisible =
      _anchorType == AnchorType::PositionIsAfterAnchor
        ? MakeDeprecatedLegacyPosition(ShareRefPtr(anchorNode),
                                       static_cast<uint32>(CaretMaxOffset(*anchorNode)))
        : *this;
    PositionIterator currentPosition = lastVisible;
    bool startEditable = startNode->HasEditableStyle();
    Node *lastNode = startNode;
    bool boundaryCrossed = false;
    for (; !currentPosition.AtStart(); currentPosition.Decrement())
    {
      auto &currentNode = *currentPosition.GetNode();

      // Don't check for an editability change if we haven't moved to a different node,
      // to avoid the expense of computing hasEditableStyle().
      if (&currentNode != lastNode)
      {
        // Don't change editability.
        bool currentEditable = currentNode.HasEditableStyle();
        if (startEditable != currentEditable)
        {
          if (rule == EditingBoundaryCrossingRule::CannotCrossEditingBoundary)
          {
            break;
          }
          boundaryCrossed = true;
        }
        lastNode = &currentNode;
      }

      // There is no caret position in non-text svg elements.
      if (IsNonTextSVGElement(currentNode))
      {
        continue;
      }

      // If we've moved to a position that is visually distinct, return the last saved position. There
      // is code below that terminates early if we're *about* to move to a visually distinct position.
      if (EndsOfNodeAreVisuallyDistinctPositions(&currentNode) && &currentNode != boundary)
      {
        return lastVisible;
      }

      // skip position in unrendered or invisible node
      auto *renderer = currentNode.Renderer();
      if (!renderer || renderer->Style().GetVisibility() != Visibility::Visible)
      {
        continue;
      }

      if (rule == EditingBoundaryCrossingRule::CanCrossEditingBoundary && boundaryCrossed)
      {
        lastVisible = currentPosition;
        break;
      }

      // track last visible streamer position
      if (IsStreamer(currentPosition))
      {
        lastVisible = currentPosition;
      }

      // Don't move past a position that is visually distinct.  We could rely on code above to terminate and
      // return lastVisible on the next iteration, but we terminate early to avoid doing a computeNodeIndex()
      // call.
      if (EndsOfNodeAreVisuallyDistinctPositions(&currentNode) && currentPosition.AtStartOfNode())
      {
        return lastVisible;
      }

      // Return position after tables and nodes which have content that can be ignored.
      if (EditingIgnoresContent(currentNode) || IsRenderedTable(&currentNode))
      {
        if (currentPosition.AtEndOfNode())
        {
          return PositionAfterNode(&currentNode);
        }
        continue;
      }

      // return current position if it is in rendered text
      if (auto *textRenderer = DynamicDowncast<RenderText>(*renderer))
      {
        auto [firstTextBox, orderCache] = inline_iterator::FirstTextBoxInLogicalOrderFor(*textRenderer);
        if (!firstTextBox)
        {
          continue;
        }

        if (&currentNode != startNode)
        {
          // This assertion fires in layout tests in the case-transform.html test because
          // of a mix-up between offsets in the text in the DOM tree with text in the
          // render tree which can have a different length due to case transformation.
          // Until we resolve that, disable this so we can run the layout tests!
          // ASSERT(currentOffset >= renderer->caretMaxOffset());
          return MakeDeprecatedLegacyPosition(ShareRefPtr(&currentNode),
                                              static_cast<uint32>(renderer->CaretMaxOffset()));
        }

        uint32 textOffset = static_cast<uint32>(currentPosition.OffsetInLeafNode());
        for (auto box = firstTextBox; box;)
        {
          if (textOffset > box->Start() && textOffset <= box->End())
          {
            return currentPosition;
          }

          auto nextBox = inline_iterator::NextTextBoxInLogicalOrder(box, orderCache);
          if (textOffset == box->End() + 1 && nextBox && box->GetLineBox() != nextBox->GetLineBox())
          {
            return currentPosition;
          }

          box = nextBox;
        }
      }
    }

    return lastVisible;
  }

  // This function and upstream() are used for moving back and forth between visually equivalent candidates.
  // For example, for the text node "foo     bar" where whitespace is collapsible, there are two candidates
  // that map to the VisiblePosition between 'b' and the space.  This function will return the right candidate
  // and upstream() will return the left one.
  // Also, downstream() will return the last position in the last atomic node in boundary for all of the
  // positions in boundary after the last candidate, where endsOfNodeAreVisuallyDistinctPositions(boundary).
  // FIXME: This function should never be called when the line box tree is dirty. See
  // https://bugs.webkit.org/show_bug.cgi?id=97264
  Position Position::Downstream(EditingBoundaryCrossingRule rule) const noexcept
  {
    auto *startNode = DeprecatedNode();
    if (!startNode)
    {
      return {};
    }

    // iterate forward from there, looking for a qualified position
    auto *boundary = EnclosingVisualBoundary(startNode);
    // FIXME: PositionIterator should respect Before and After positions.
    auto *anchorNode = _anchorNode.get();
    PositionIterator lastVisible =
      _anchorType == AnchorType::PositionIsAfterAnchor
        ? MakeDeprecatedLegacyPosition(ShareRefPtr(anchorNode),
                                       static_cast<uint32>(CaretMaxOffset(*anchorNode)))
        : *this;
    PositionIterator currentPosition = lastVisible;
    bool startEditable = startNode->HasEditableStyle();
    Node *lastNode = startNode;
    bool boundaryCrossed = false;
    for (; !currentPosition.AtEnd(); currentPosition.Increment())
    {
      auto &currentNode = *currentPosition.GetNode();

      // Don't check for an editability change if we haven't moved to a different node,
      // to avoid the expense of computing hasEditableStyle().
      if (&currentNode != lastNode)
      {
        // Don't change editability.
        bool currentEditable = currentNode.HasEditableStyle();
        if (startEditable != currentEditable)
        {
          if (rule == EditingBoundaryCrossingRule::CannotCrossEditingBoundary)
          {
            break;
          }
          boundaryCrossed = true;
        }

        lastNode = &currentNode;
      }

      // stop before going above the body, up into the head
      // return the last visible streamer position
      if (auto *element = DynamicDowncast<html::HTMLElement>(currentNode);
          element && Is<html::HTMLBodyElement>(*element) && currentPosition.AtEndOfNode())
      {
        break;
      }

      // There is no caret position in non-text svg elements.
      if (IsNonTextSVGElement(currentNode))
      {
        continue;
      }

      // Do not move to a visually distinct position.
      if (EndsOfNodeAreVisuallyDistinctPositions(&currentNode) && &currentNode != boundary)
      {
        return lastVisible;
      }
      // Do not move past a visually disinct position.
      // Note: The first position after the last in a node whose ends are visually distinct
      // positions will be [boundary->parentNode(), originalBlock->computeNodeIndex() + 1].
      if (boundary && boundary->ParentNode() == &currentNode)
      {
        return lastVisible;
      }

      // skip position in unrendered or invisible node
      auto *renderer = currentNode.Renderer();
      if (!renderer || renderer->Style().GetVisibility() != Visibility::Visible)
      {
        continue;
      }

      if (rule == EditingBoundaryCrossingRule::CanCrossEditingBoundary && boundaryCrossed)
      {
        lastVisible = currentPosition;
        break;
      }

      // track last visible streamer position
      if (IsStreamer(currentPosition))
      {
        lastVisible = currentPosition;
      }

      // Return position before tables and nodes which have content that can be ignored.
      if (EditingIgnoresContent(currentNode) || IsRenderedTable(&currentNode))
      {
        if (currentPosition.AtStartOfNode())
        {
          return PositionBeforeNode(&currentNode);
        }
        continue;
      }

      // return current position if it is in rendered text
      if (auto *textRenderer = DynamicDowncast<RenderText>(*renderer))
      {
        auto [firstTextBox, orderCache] = inline_iterator::FirstTextBoxInLogicalOrderFor(*textRenderer);
        if (!firstTextBox)
        {
          continue;
        }

        if (&currentNode != startNode)
        {
          krys_debug_assert(currentPosition.AtStartOfNode());
          return MakeContainerOffsetPosition(ShareRefPtr(&currentNode),
                                             static_cast<uint32>(textRenderer->CaretMinOffset()));
        }

        uint32 textOffset = static_cast<uint32>(currentPosition.OffsetInLeafNode());
        for (auto box = firstTextBox; box;)
        {
          if (!box->Length() && textOffset == box->Start())
          {
            return currentPosition;
          }

          if (textOffset >= box->Start() && textOffset < box->End())
          {
            return currentPosition;
          }

          auto nextBox = inline_iterator::NextTextBoxInLogicalOrder(box, orderCache);
          if (textOffset == box->End() && nextBox && box->GetLineBox() != nextBox->GetLineBox())
          {
            return currentPosition;
          }

          box = nextBox;
        }
      }
    }

    return lastVisible;
  }

  uint32 Position::PositionCountBetweenPositions(const Position &a, const Position &b) noexcept
  {
    if (a.IsNull() || b.IsNull())
    {
      return UINT_MAX;
    }

    Position endPos;
    Position pos;
    if (a > b)
    {
      endPos = a;
      pos = b;
    }
    else if (a < b)
    {
      endPos = b;
      pos = a;
    }
    else
    {
      return 0;
    }

    uint32 posCount = 0;
    while (!pos.AtEndOfTree() && pos != endPos)
    {
      pos = pos.Next();
      ++posCount;
    }
    return posCount;
  }

  bool Position::HasRenderedNonAnonymousDescendantsWithHeight(const RenderElement &renderer) noexcept
  {
    auto isHorizontal = renderer.IsHorizontalWritingMode();
    auto *stop = renderer.NextInPreOrderAfterChildren();
    for (auto *descendant = renderer.FirstChild(); descendant && descendant != stop;
         descendant = descendant->NextInPreOrder())
    {
      if (!descendant->NonPseudoNode())
      {
        continue;
      }

      auto boundingBoxLogicalHeight = [&](const IntRect &rect)
      {
        return isHorizontal ? rect.Height() : rect.Width();
      };

      if (auto *renderText = DynamicDowncast<RenderText>(*descendant))
      {
        if (boundingBoxLogicalHeight(renderText->LinesBoundingBox()))
        {
          return true;
        }
        continue;
      }
      if (auto *renderLineBreak = DynamicDowncast<RenderLineBreak>(*descendant))
      {
        if (boundingBoxLogicalHeight(renderLineBreak->LinesBoundingBox()))
        {
          return true;
        }
        continue;
      }
      if (auto *renderInline = DynamicDowncast<RenderInline>(*descendant))
      {
        if (IsEmptyInline(*renderInline) && boundingBoxLogicalHeight(renderInline->LinesBoundingBox()))
        {
          return true;
        }
        continue;
      }
      if (auto *renderBox = DynamicDowncast<RenderBox>(*descendant))
      {
        if (RoundToInt(renderBox->LogicalHeight()))
        {
          return true;
        }
        continue;
      }
    }
    return false;
  }

  bool Position::NodeIsUserSelectNone(const Node *node) noexcept
  {
    if (!node)
    {
      return false;
    }
    return node->Renderer() && (node->Renderer()->Style().UsedUserSelect() == UserSelect::None);
  }

  bool Position::NodeIsUserSelectAll(const Node *node) noexcept
  {
    if (!node)
    {
      return false;
    }
    auto *renderer = node->Renderer();
    return renderer && renderer->Style().UsedUserSelect() == UserSelect::All;
  }

  RefPtr<Node> Position::RootUserSelectAllForNode(Node *node) noexcept
  {
    if (!node || !NodeIsUserSelectAll(node))
    {
      return nullptr;
    }
    Node *parent = node->ParentNode();
    if (!parent)
    {
      return ShareRefPtr(node);
    }

    Node *candidateRoot = node;
    while (parent)
    {
      if (!parent->Renderer())
      {
        parent = parent->ParentNode();
        continue;
      }
      if (!NodeIsUserSelectAll(parent))
      {
        break;
      }
      candidateRoot = parent;
      parent = candidateRoot->ParentNode();
    }
    return ShareRefPtr(candidateRoot);
  }

  // This function should be kept in sync with PositionIterator::IsCandidate().
  bool Position::IsCandidate() const noexcept
  {
    if (IsNull())
    {
      return false;
    }

    auto *node = DeprecatedNode();
    auto *renderer = node->Renderer();
    if (!renderer)
    {
      return false;
    }

    if (renderer->Style().GetVisibility() != Visibility::Visible)
    {
      return false;
    }

    if (renderer->IsBR())
    {
      // FIXME: The condition should be _anchorType == PositionIsBeforeAnchor, but for now we still need to
      // support legacy positions.
      return !_offset && _anchorType != AnchorType::PositionIsAfterAnchor
             && !NodeIsUserSelectNone(node->ParentNode());
    }

    if (auto *renderText = DynamicDowncast<RenderText>(*renderer))
    {
      return !NodeIsUserSelectNone(node) && renderText->ContainsCaretOffset(_offset);
    }

    if (PositionBeforeOrAfterNodeIsCandidate(*node))
    {
      return ((AtFirstEditingPositionForNode() && _anchorType == AnchorType::PositionIsBeforeAnchor)
              || (AtLastEditingPositionForNode() && _anchorType == AnchorType::PositionIsAfterAnchor))
             && !NodeIsUserSelectNone(node->ParentNode());
    }

    if (Is<html::HTMLHtmlElement>(*_anchorNode))
    {
      return false;
    }

    if (auto *block = DynamicDowncast<RenderBlock>(*renderer))
    {
      if (Is<RenderBlockFlow>(*block) || Is<RenderGrid>(*block) || Is<RenderFlexibleBox>(*block))
      {
        if (block->LogicalHeight() || Is<html::HTMLBodyElement>(*_anchorNode)
            || _anchorNode->IsRootEditableElement())
        {
          if (!HasRenderedNonAnonymousDescendantsWithHeight(*block))
          {
            return AtFirstEditingPositionForNode() && !NodeIsUserSelectNone(node);
          }
          return _anchorNode->HasEditableStyle() && !NodeIsUserSelectNone(node) && AtEditingBoundary();
        }
        return false;
      }
    }

    return _anchorNode->HasEditableStyle() && !NodeIsUserSelectNone(node) && AtEditingBoundary();
  }

  bool Position::IsRenderedCharacter() const noexcept
  {
    auto *text = DynamicDowncast<Text>(DeprecatedNode());
    auto *renderer = text ? text->Renderer() : nullptr;
    return renderer && renderer->ContainsRenderedCharacterOffset(_offset);
  }

  namespace
  {
    KRYS_NODISCARD bool InSameEnclosingBlockFlowElement(Node *a, Node *b) noexcept
    {
      return a && b && DeprecatedEnclosingBlockFlowElement(a) == DeprecatedEnclosingBlockFlowElement(b);
    }

    KRYS_NODISCARD bool IsHTMLBRElement(const Node &node) noexcept
    {
      auto *element = DynamicDowncast<html::HTMLElement>(node);
      return element && Is<html::HTMLBRElement>(*element);
    }
  }

  bool Position::RendersInDifferentPosition(const Position &position) const noexcept
  {
    if (IsNull() || position.IsNull())
    {
      return false;
    }

    auto *node = DeprecatedNode();
    auto *renderer = node->Renderer();
    if (!renderer)
    {
      return false;
    }

    auto *positionNode = position.DeprecatedNode();
    auto *positionRenderer = positionNode->Renderer();
    if (!positionRenderer)
    {
      return false;
    }

    if (renderer->Style().GetVisibility() != Visibility::Visible
        || positionRenderer->Style().GetVisibility() != Visibility::Visible)
    {
      return false;
    }

    if (node == positionNode)
    {
      if (IsHTMLBRElement(*node))
      {
        return false;
      }

      if (static_cast<int32>(_offset) == position.DeprecatedEditingOffset())
      {
        return false;
      }

      if (!Is<Text>(*node))
      {
        return true;
      }
    }

    if (IsHTMLBRElement(*node) && position.IsCandidate())
    {
      return true;
    }

    if (IsHTMLBRElement(*positionNode) && IsCandidate())
    {
      return true;
    }

    if (!InSameEnclosingBlockFlowElement(node, positionNode))
    {
      return true;
    }

    auto *textRenderer = DynamicDowncast<RenderText>(*renderer);
    if (textRenderer && !textRenderer->ContainsCaretOffset(_offset))
    {
      return false;
    }

    auto *textPositionRenderer = DynamicDowncast<RenderText>(*positionRenderer);
    if (textPositionRenderer && !textPositionRenderer->ContainsCaretOffset(position._offset))
    {
      return false;
    }

    uint32 thisRenderedOffset =
      textRenderer ? textRenderer->CountRenderedCharacterOffsetsUntil(_offset) : _offset;
    uint32 positionRenderedOffset =
      textPositionRenderer ? textPositionRenderer->CountRenderedCharacterOffsetsUntil(position._offset)
                           : position._offset;

    if (renderer == positionRenderer && thisRenderedOffset == positionRenderedOffset)
    {
      return false;
    }

    auto box1 = GetInlineBoxAndOffset(Affinity::Downstream).Box;
    auto box2 = position.GetInlineBoxAndOffset(Affinity::Downstream).Box;

    if (!box1 || !box2)
    {
      return false;
    }

    if (box1->GetLineBox() != box2->GetLineBox())
    {
      return true;
    }

    if (NextRenderedEditable(node) == positionNode
        && thisRenderedOffset == static_cast<uint32>(CaretMaxOffset(*positionNode))
        && !positionRenderedOffset)
    {
      return false;
    }

    if (PreviousRenderedEditable(node) == positionNode && !thisRenderedOffset
        && positionRenderedOffset == static_cast<uint32>(CaretMaxOffset(*positionNode)))
    {
      return false;
    }

    return true;
  }

  // This assumes that it starts in editable content.
  Position Position::LeadingWhitespacePosition(Affinity affinity,
                                               bool considerNonCollapsibleWhitespace) const noexcept
  {
    krys_debug_assert(IsEditablePosition(*this));
    if (IsNull())
    {
      return {};
    }

    if (IsHTMLBRElement(*Upstream().DeprecatedNode()))
    {
      return {};
    }

    Position prev = PreviousCharacterPosition(affinity);
    auto *node = DeprecatedNode();
    auto *previousNode = prev.DeprecatedNode();
    if (prev != *this && InSameEnclosingBlockFlowElement(node, previousNode))
    {
      if (auto *previousText = DynamicDowncast<Text>(*previousNode))
      {
        char32 c = CodePointAt(*previousText, static_cast<uint32>(prev.DeprecatedEditingOffset()));
        if (considerNonCollapsibleWhitespace ? (text::IsASCIIWhitespace(c) || c == 0xA0)
                                             : DeprecatedIsCollapsibleWhitespace(c))
        {
          if (IsEditablePosition(prev))
          {
            return prev;
          }
        }
      }
    }

    return {};
  }

  // This assumes that it starts in editable content.
  Position Position::TrailingWhitespacePosition(Affinity,
                                                bool considerNonCollapsibleWhitespace) const noexcept
  {
    krys_debug_assert(IsEditablePosition(*this));
    if (IsNull())
    {
      return {};
    }

    VisiblePosition v(*this);
    char32 c = v.CharacterAfter();
    // The space must not be in another paragraph and it must be editable.
    if (!IsEndOfParagraph(v) && v.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary).IsNotNull())
    {
      if (considerNonCollapsibleWhitespace ? (text::IsASCIIWhitespace(c) || c == 0xA0)
                                           : DeprecatedIsCollapsibleWhitespace(c))
      {
        return *this;
      }
    }

    return {};
  }

  InlineBoxAndOffset Position::GetInlineBoxAndOffset(Affinity affinity) const noexcept
  {
    return GetInlineBoxAndOffset(affinity, PrimaryDirection());
  }

  namespace
  {
    KRYS_NODISCARD bool IsNonTextLeafChild(const RenderObject &object) noexcept
    {
      if (auto *renderElement = DynamicDowncast<RenderElement>(object))
      {
        return !renderElement->FirstChild();
      }
      return false;
    }

    KRYS_NODISCARD inline_iterator::TextBoxIterator
      SearchAheadForBetterMatch(const RenderText &renderer) noexcept
    {
      auto *container = renderer.ContainingBlock();
      const RenderObject *next = &renderer;
      while ((next = next->NextInPreOrder(container)))
      {
        if (Is<RenderBlock>(*next))
        {
          return {};
        }
        if (next->IsBR())
        {
          return {};
        }
        if (IsNonTextLeafChild(*next))
        {
          return {};
        }
        if (auto *renderText = DynamicDowncast<RenderText>(*next))
        {
          if (auto box = inline_iterator::FirstTextBoxInLogicalOrderFor(*renderText).first)
          {
            return box;
          }
        }
      }
      return {};
    }

    KRYS_NODISCARD Position DownstreamIgnoringEditingBoundaries(Position position) noexcept
    {
      Position lastPosition;
      while (position != lastPosition)
      {
        lastPosition = position;
        position = position.Downstream(EditingBoundaryCrossingRule::CanCrossEditingBoundary);
      }
      return position;
    }

    KRYS_NODISCARD Position UpstreamIgnoringEditingBoundaries(Position position) noexcept
    {
      Position lastPosition;
      while (position != lastPosition)
      {
        lastPosition = position;
        position = position.Upstream(EditingBoundaryCrossingRule::CanCrossEditingBoundary);
      }
      return position;
    }
  }

  InlineBoxAndOffset Position::GetInlineBoxAndOffset(Affinity affinity,
                                                     TextDirection primaryDirection) const noexcept
  {
    auto caretOffset = static_cast<uint32>(DeprecatedEditingOffset());

    auto *node = DeprecatedNode();
    if (!node)
    {
      return {{}, caretOffset};
    }
    auto *renderer = node->Renderer();
    if (!renderer)
    {
      return {{}, caretOffset};
    }

    inline_iterator::LeafBoxIterator box;

    if (auto *lineBreakRenderer = DynamicDowncast<RenderLineBreak>(*renderer);
        lineBreakRenderer && lineBreakRenderer->IsBR())
    {
      if (!caretOffset)
      {
        box = inline_iterator::BoxFor(*lineBreakRenderer);
      }
    }
    else if (auto *textRenderer = DynamicDowncast<RenderText>(*renderer))
    {
      auto textBox = inline_iterator::LineLeftmostTextBoxFor(*textRenderer);
      inline_iterator::TextBoxIterator candidate;

      for (; textBox; ++textBox)
      {
        uint32 caretMinOffset = textBox->MinimumCaretOffset();
        uint32 caretMaxOffset = textBox->MaximumCaretOffset();

        if (caretOffset < caretMinOffset || caretOffset > caretMaxOffset
            || (caretOffset == caretMaxOffset && textBox->IsLineBreak()))
        {
          continue;
        }

        if (caretOffset > caretMinOffset && caretOffset < caretMaxOffset)
        {
          return {textBox, caretOffset};
        }

        // Boxes come line by line, from line-left to line-right. Where two boxes on a line meet at the caret,
        // it belongs to the leftmost, whichever way the boxes run; affinity only chooses between lines.
        if (candidate && candidate->LineIndex() == textBox->LineIndex())
        {
          continue;
        }

        if ((caretOffset == caretMaxOffset) ^ (affinity == Affinity::Downstream))
        {
          break;
        }

        if ((caretOffset == caretMinOffset) ^ (affinity == Affinity::Upstream))
        {
          break;
        }

        if (caretOffset == caretMaxOffset)
        {
          auto nextOnLine = textBox->NextLineRightwardOnLine();
          if (nextOnLine && nextOnLine->IsLineBreak())
          {
            break;
          }
        }

        candidate = textBox;
      }

      if (candidate && !candidate->NextTextBox() && affinity == Affinity::Downstream)
      {
        textBox = SearchAheadForBetterMatch(*textRenderer);
        if (textBox && textBox->LineIndex() == candidate->LineIndex()
            && textBox->LogicalLeftIgnoringInlineDirection()
                 > candidate->LogicalLeftIgnoringInlineDirection())
        {
          textBox = {};
        }
        if (textBox)
        {
          caretOffset = textBox->MinimumCaretOffset();
        }
      }

      box = textBox ? inline_iterator::LeafBoxIterator(textBox) : inline_iterator::LeafBoxIterator(candidate);
    }
    else
    {
      if (CanHaveChildrenForEditing(*node))
      {
        auto *renderBlockFlow = DynamicDowncast<RenderBlockFlow>(*renderer);
        if (renderBlockFlow && HasRenderedNonAnonymousDescendantsWithHeight(*renderBlockFlow))
        {
          // Try a visually equivalent position with possibly opposite editability. This helps in case |this|
          // is in an editable block but surrounded by non-editable positions. It acts to negate the logic at
          // the beginning of RenderObject::createPositionWithAffinity().
          Position equivalent = DownstreamIgnoringEditingBoundaries(*this);
          if (equivalent == *this)
          {
            equivalent = UpstreamIgnoringEditingBoundaries(*this);
            // FIXME: Can returning nullptr really be correct here?
            if (equivalent == *this || DownstreamIgnoringEditingBoundaries(equivalent) == *this)
            {
              return {{}, caretOffset};
            }
          }

          return equivalent.GetInlineBoxAndOffset(Affinity::Upstream, primaryDirection);
        }
      }
      if (auto *renderBox = DynamicDowncast<RenderBox>(*renderer))
      {
        box = inline_iterator::BoxFor(*renderBox);
        if (box && caretOffset > box->MinimumCaretOffset() && caretOffset < box->MaximumCaretOffset())
        {
          return {box, caretOffset};
        }
      }
    }

    if (!box)
    {
      return {{}, caretOffset};
    }

    return {box, caretOffset};
  }

  TextDirection Position::PrimaryDirection() const noexcept
  {
    if (!_anchorNode || !_anchorNode->Renderer())
    {
      return TextDirection::LTR;
    }
    for (auto *renderer = _anchorNode->Renderer(); renderer; renderer = renderer->Parent())
    {
      if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(*renderer))
      {
        return blockFlow->Style().GetWritingMode().BidiDirection();
      }
    }
    return TextDirection::LTR;
  }

  bool Position::Equals(const Position &other) const noexcept
  {
    if (!_anchorNode)
    {
      return !_anchorNode == !other._anchorNode;
    }
    if (!other._anchorNode)
    {
      return false;
    }

    switch (GetAnchorType())
    {
      case AnchorType::PositionIsBeforeChildren:
        krys_debug_assert(!Is<Text>(*_anchorNode));
        switch (other.GetAnchorType())
        {
          case AnchorType::PositionIsBeforeChildren:
            krys_debug_assert(!Is<Text>(*other._anchorNode));
            return _anchorNode == other._anchorNode;
          case AnchorType::PositionIsAfterChildren:
            krys_debug_assert(!Is<Text>(*other._anchorNode));
            return _anchorNode == other._anchorNode && !_anchorNode->HasChildNodes();
          case AnchorType::PositionIsOffsetInAnchor:
            return _anchorNode == other._anchorNode && !other._offset;
          case AnchorType::PositionIsBeforeAnchor:
            return _anchorNode->FirstChild() == other._anchorNode.get();
          case AnchorType::PositionIsAfterAnchor: return false;
        }
        break;
      case AnchorType::PositionIsAfterChildren:
        krys_debug_assert(!Is<Text>(*_anchorNode));
        switch (other.GetAnchorType())
        {
          case AnchorType::PositionIsBeforeChildren:
            krys_debug_assert(!Is<Text>(*other._anchorNode));
            return _anchorNode == other._anchorNode && !_anchorNode->HasChildNodes();
          case AnchorType::PositionIsAfterChildren:
            krys_debug_assert(!Is<Text>(*other._anchorNode));
            return _anchorNode == other._anchorNode;
          case AnchorType::PositionIsOffsetInAnchor:
            return _anchorNode == other._anchorNode && _anchorNode->CountChildNodes() == _offset;
          case AnchorType::PositionIsBeforeAnchor: return false;
          case AnchorType::PositionIsAfterAnchor:  return _anchorNode->LastChild() == other._anchorNode.get();
        }
        break;
      case AnchorType::PositionIsOffsetInAnchor:
        switch (other.GetAnchorType())
        {
          case AnchorType::PositionIsBeforeChildren:
            krys_debug_assert(!Is<Text>(*other._anchorNode));
            return _anchorNode == other._anchorNode && !_offset;
          case AnchorType::PositionIsAfterChildren:
            krys_debug_assert(!Is<Text>(*other._anchorNode));
            return _anchorNode == other._anchorNode && _offset == other._anchorNode->CountChildNodes();
          case AnchorType::PositionIsOffsetInAnchor:
            return _anchorNode == other._anchorNode && _offset == other._offset;
          case AnchorType::PositionIsBeforeAnchor:
            return _anchorNode->TraverseToChildAt(_offset) == other._anchorNode.get();
          case AnchorType::PositionIsAfterAnchor:
            return _offset && _anchorNode->TraverseToChildAt(_offset - 1) == other._anchorNode.get();
        }
        break;
      case AnchorType::PositionIsBeforeAnchor:
        switch (other.GetAnchorType())
        {
          case AnchorType::PositionIsBeforeChildren:
            krys_debug_assert(!Is<Text>(*other._anchorNode));
            return _anchorNode.get() == other._anchorNode->FirstChild();
          case AnchorType::PositionIsAfterChildren:
            krys_debug_assert(!Is<Text>(*other._anchorNode));
            return false;
          case AnchorType::PositionIsOffsetInAnchor:
            return _anchorNode.get() == other._anchorNode->TraverseToChildAt(other._offset);
          case AnchorType::PositionIsBeforeAnchor: return _anchorNode == other._anchorNode;
          case AnchorType::PositionIsAfterAnchor:
            return _anchorNode->PreviousSibling() == other._anchorNode.get();
        }
        break;
      case AnchorType::PositionIsAfterAnchor:
        switch (other.GetAnchorType())
        {
          case AnchorType::PositionIsBeforeChildren:
            krys_debug_assert(!Is<Text>(*other._anchorNode));
            return false;
          case AnchorType::PositionIsAfterChildren:
            krys_debug_assert(!Is<Text>(*other._anchorNode));
            return _anchorNode.get() == other._anchorNode->LastChild();
          case AnchorType::PositionIsOffsetInAnchor:
            return other._offset
                   && _anchorNode.get() == other._anchorNode->TraverseToChildAt(other._offset - 1);
          case AnchorType::PositionIsBeforeAnchor:
            return _anchorNode->NextSibling() == other._anchorNode.get();
          case AnchorType::PositionIsAfterAnchor: return _anchorNode == other._anchorNode;
        }
        break;
    }

    krys_unreachable();
  }

  Node *CommonInclusiveAncestor(const Position &a, const Position &b) noexcept
  {
    auto *nodeA = a.GetContainerNode();
    auto *nodeB = b.GetContainerNode();
    if (!nodeA || !nodeB)
    {
      return nullptr;
    }
    return CommonInclusiveAncestor<TreeType::ComposedTree>(*nodeA, *nodeB);
  }

  Position PositionInParentBeforeNode(Node *node) noexcept
  {
    Node *currentNode = node;
    Node *ancestor = node->ParentNode();
    while (ancestor && EditingIgnoresContent(*ancestor))
    {
      currentNode = ancestor;
      ancestor = ancestor->ParentNode();
    }
    krys_debug_assert(ancestor);
    return Position(ShareRefPtr(ancestor), currentNode->ComputeNodeIndex(),
                    Position::AnchorType::PositionIsOffsetInAnchor);
  }

  Position PositionInParentAfterNode(Node *node) noexcept
  {
    Node *currentNode = node;
    Node *ancestor = node->ParentNode();
    while (ancestor && EditingIgnoresContent(*ancestor))
    {
      currentNode = ancestor;
      ancestor = ancestor->ParentNode();
    }
    krys_debug_assert(ancestor);
    return Position(ShareRefPtr(ancestor), currentNode->ComputeNodeIndex() + 1,
                    Position::AnchorType::PositionIsOffsetInAnchor);
  }

  Position FirstPositionInNode(Node *anchorNode) noexcept
  {
    if (Is<CharacterData>(*anchorNode))
    {
      return Position(ShareRefPtr(anchorNode), 0, Position::AnchorType::PositionIsOffsetInAnchor);
    }
    return Position(ShareRefPtr(anchorNode), Position::AnchorType::PositionIsBeforeChildren);
  }

  Position LastPositionInNode(Node *anchorNode) noexcept
  {
    if (Is<CharacterData>(*anchorNode))
    {
      return Position(ShareRefPtr(anchorNode), anchorNode->Length(),
                      Position::AnchorType::PositionIsOffsetInAnchor);
    }
    return Position(ShareRefPtr(anchorNode), Position::AnchorType::PositionIsAfterChildren);
  }

  bool OffsetIsBeforeLastNodeOffset(uint32 offset, Node *anchorNode) noexcept
  {
    if (Is<CharacterData>(*anchorNode))
    {
      return offset < anchorNode->Length();
    }

    uint32 currentOffset = 0;
    for (auto *node = anchorNode->FirstChild(); node && currentOffset < offset; node = node->NextSibling())
    {
      currentOffset++;
    }

    return offset < currentOffset;
  }

  Position MakeContainerOffsetPosition(const BoundaryPoint &point) noexcept
  {
    return MakeContainerOffsetPosition(ShareRefPtr(point.Container.get()), static_cast<uint32>(point.Offset));
  }

  Position MakeDeprecatedLegacyPosition(RefPtr<Node> &&node, uint32 offset) noexcept
  {
    return {krys::move(node), offset, Position::LegacyEditingPositionFlag::On};
  }

  Position MakeDeprecatedLegacyPosition(const BoundaryPoint &point) noexcept
  {
    return MakeDeprecatedLegacyPosition(ShareRefPtr(point.Container.get()),
                                        static_cast<uint32>(point.Offset));
  }

  Maybe<BoundaryPoint> MakeBoundaryPoint(const Position &position) noexcept
  {
    auto *container = position.GetContainerNode();
    if (!container)
    {
      return null;
    }
    return BoundaryPoint {ShareRef(*container), static_cast<uint64>(position.ComputeOffsetInContainerNode())};
  }

  Maybe<SimpleRange> MakeSimpleRange(const PositionRange &range) noexcept
  {
    return MakeSimpleRange(range.Start, range.End);
  }

  template <TreeType treeType>
  std::partial_ordering TreeOrder(const Position &a, const Position &b) noexcept
  {
    if (a.IsNull() || b.IsNull())
    {
      return a.IsNull() && b.IsNull() ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }

    auto *aContainer = a.GetContainerNode();
    auto *bContainer = b.GetContainerNode();

    if (!aContainer || !bContainer)
    {
      if (!CommonInclusiveAncestor<treeType>(*a.AnchorNode(), *b.AnchorNode()))
      {
        return std::partial_ordering::unordered;
      }
      if (!aContainer && !bContainer && a.GetAnchorType() == b.GetAnchorType())
      {
        return std::partial_ordering::equivalent;
      }
      if (bContainer)
      {
        return a.GetAnchorType() == Position::AnchorType::PositionIsBeforeAnchor
                 ? std::partial_ordering::less
                 : std::partial_ordering::greater;
      }
      return b.GetAnchorType() == Position::AnchorType::PositionIsBeforeAnchor
               ? std::partial_ordering::greater
               : std::partial_ordering::less;
    }

    // FIXME: Avoid computing node offset for cases where we don't need to.

    return TreeOrder<treeType>(*MakeBoundaryPoint(a), *MakeBoundaryPoint(b));
  }

  std::partial_ordering operator<=>(const Position &a, const Position &b) noexcept
  {
    return TreeOrder<TreeType::ComposedTree>(a, b);
  }

  template std::partial_ordering TreeOrder<TreeType::ComposedTree>(const Position &,
                                                                   const Position &) noexcept;
  template std::partial_ordering TreeOrder<TreeType::ShadowIncludingTree>(const Position &,
                                                                          const Position &) noexcept;
}

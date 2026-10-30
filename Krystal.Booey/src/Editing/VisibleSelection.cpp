#include "Krystal.Booey/Editing/VisibleSelection.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <utility>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool IsInUserAgentShadowRootOrHasEditableShadowAncestor(dom::Node &node) noexcept
    {
      auto *shadowRoot = dom::TreeQueries::ContainingShadowRoot(node);
      if (!shadowRoot)
      {
        return false;
      }

      if (shadowRoot->Mode() == dom::ShadowRootMode::UserAgent)
      {
        return true;
      }

      for (dom::Node *currentNode = &node; currentNode; currentNode = currentNode->ParentOrShadowHostNode())
      {
        if (currentNode->HasEditableStyle())
        {
          return true;
        }
      }
      return false;
    }
  }

  VisibleSelection::VisibleSelection(const dom::Position &anchor, const dom::Position &focus,
                                     Affinity affinity, boo::Directionality directionality) noexcept
      : _anchor(anchor), _focus(focus), _affinity(affinity), _directionality(directionality)
  {
    Validate();
  }

  VisibleSelection::VisibleSelection(const dom::Position &position, Affinity affinity,
                                     boo::Directionality directionality) noexcept
      : VisibleSelection(position, position, affinity, directionality)
  {
  }

  VisibleSelection::VisibleSelection(const VisiblePosition &position,
                                     boo::Directionality directionality) noexcept
      : VisibleSelection(position.DeepEquivalent(), position.GetAffinity(), directionality)
  {
    // FIXME: Wasteful that this re-canonicalizes, but risky to change since the VisiblePosition object could
    // be from before a mutation and its position may no longer be canonical.
  }

  VisibleSelection::VisibleSelection(const VisiblePosition &anchor, const VisiblePosition &focus,
                                     boo::Directionality directionality) noexcept
      : VisibleSelection(anchor.DeepEquivalent(), focus.DeepEquivalent(), anchor.GetAffinity(),
                         directionality)
  {
    // FIXME: Wasteful that this re-canonicalizes, but risky to change since the VisiblePosition objects could
    // be from before a mutation and their positions may no longer be canonical.
  }

  VisibleSelection::VisibleSelection(const dom::SimpleRange &range, Affinity affinity,
                                     boo::Directionality directionality) noexcept
      : VisibleSelection(dom::MakeDeprecatedLegacyPosition(range.Start),
                         dom::MakeDeprecatedLegacyPosition(range.End), affinity, directionality)
  {
  }

  const VisibleSelection &VisibleSelection::EmptySelection() noexcept
  {
    static NeverDestroyed<VisibleSelection> selection;
    return selection.get();
  }

  VisibleSelection VisibleSelection::SelectionFromContentsOfNode(dom::Node *node) noexcept
  {
    krys_debug_assert(!EditingIgnoresContent(*node));
    return VisibleSelection(VisiblePosition {dom::FirstPositionInNode(node)},
                            VisiblePosition {dom::LastPositionInNode(node)});
  }

  const dom::Position &VisibleSelection::UncanonicalizedStart() const noexcept
  {
    return _anchorIsFirst ? _anchor : _focus;
  }

  const dom::Position &VisibleSelection::UncanonicalizedEnd() const noexcept
  {
    return _anchorIsFirst ? _focus : _anchor;
  }

  Maybe<dom::SimpleRange> VisibleSelection::Range() const noexcept
  {
    const auto &start = UncanonicalizedStart();
    const auto &end = UncanonicalizedEnd();
    if (start.GetDocument())
    {
      return dom::MakeSimpleRange({start, end});
    }
    return dom::MakeSimpleRange({start.ParentAnchoredEquivalent(), end.ParentAnchoredEquivalent()});
  }

  void VisibleSelection::SetBase(const dom::Position &position) noexcept
  {
    _anchor = position;
    Validate();
  }

  void VisibleSelection::SetBase(const VisiblePosition &visiblePosition) noexcept
  {
    SetBase(visiblePosition.DeepEquivalent());
  }

  void VisibleSelection::SetExtent(const dom::Position &position) noexcept
  {
    _focus = position;
    Validate();
  }

  void VisibleSelection::SetExtent(const VisiblePosition &visiblePosition) noexcept
  {
    SetExtent(visiblePosition.DeepEquivalent());
  }

  bool VisibleSelection::IsOrphan() const noexcept
  {
    if (_base.IsOrphan() || _extent.IsOrphan() || _start.IsOrphan() || _end.IsOrphan())
    {
      return true;
    }
    if (_anchor.IsOrphan())
    {
      return true;
    }
    if (_focus.IsOrphan())
    {
      return true;
    }
    return false;
  }

  RefPtr<dom::Document> VisibleSelection::GetDocument() const noexcept
  {
    auto *document = _base.GetDocument();
    if (!document)
    {
      document = _anchor.GetDocument();
      if (!document)
      {
        return nullptr;
      }
    }

    if (_extent.GetDocument() != document || _start.GetDocument() != document
        || _end.GetDocument() != document)
    {
      return nullptr;
    }

    if (_anchor.GetDocument() != document || _focus.GetDocument() != document)
    {
      return nullptr;
    }

    return ShareRefPtr(document);
  }

  Maybe<dom::SimpleRange> VisibleSelection::FirstRange() const noexcept
  {
    if (IsNoneOrOrphaned())
    {
      return {};
    }
    // FIXME: Seems likely we don't need to call ParentAnchoredEquivalent here.
    return dom::MakeSimpleRange({_start.ParentAnchoredEquivalent(), _end.ParentAnchoredEquivalent()});
  }

  Maybe<dom::SimpleRange> VisibleSelection::ToNormalizedRange() const noexcept
  {
    if (IsNoneOrOrphaned())
    {
      return {};
    }

    // Make sure we have an updated layout since this function is called
    // in the course of running edit commands which modify the DOM.
    // Failing to call this can result in equivalentXXXPosition calls returning
    // incorrect results.
    _start.AnchorNode()->NodeDocument().UpdateLayout();

    // Check again, because updating layout can clear the selection.
    if (IsNoneOrOrphaned())
    {
      return {};
    }

    dom::Position s;
    dom::Position e;
    if (IsCaret())
    {
      // If the selection is a caret, move the range start upstream. This helps us match
      // the conventions of text editors tested, which make style determinations based
      // on the character before the caret, if any.
      s = _start.Upstream().ParentAnchoredEquivalent();
      e = s;
    }
    else
    {
      // If the selection is a range, select the minimum range that encompasses the selection.
      // Again, this is to match the conventions of text editors tested, which make style
      // determinations based on the first character of the selection.
      // For instance, this operation helps to make sure that the "X" selected below is the
      // only thing selected. The range should not be allowed to "leak" out to the end of the
      // previous text node, or to the beginning of the next text node, each of which has a
      // different style.
      //
      // On a treasure map, <b>X</b> marks the spot.
      //                       ^ selected
      //
      krys_debug_assert(IsRange());
      s = _start.Downstream().ParentAnchoredEquivalent();
      e = _end.Upstream().ParentAnchoredEquivalent();
      // Make sure the start is before the end.
      // The end can wind up before the start if collapsed whitespace is the only thing selected.
      if (s > e)
      {
        std::swap(s, e);
      }
    }

    return dom::MakeSimpleRange({s, e});
  }

  bool VisibleSelection::ExpandUsingGranularity(TextGranularity granularity) noexcept
  {
    if (IsNone())
    {
      return false;
    }

    Validate(granularity);
    return true;
  }

  bool VisibleSelection::IsAll(EditingBoundaryCrossingRule rule) const noexcept
  {
    return !NonBoundaryShadowTreeRootNode() && VisibleStart().Previous(rule).IsNull()
           && VisibleEnd().Next(rule).IsNull();
  }

  void VisibleSelection::AppendTrailingWhitespace() noexcept
  {
    auto *scope = DeprecatedEnclosingBlockFlowElement(_end.DeprecatedNode());
    if (!scope)
    {
      return;
    }

    auto start = dom::MakeBoundaryPoint(_end);
    if (!start)
    {
      return;
    }
    CharacterIterator charIt(dom::SimpleRange {*start, dom::MakeBoundaryPointAfterNodeContents(*scope)},
                             TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions);
    for (; !charIt.AtEnd() && !charIt.Text().empty(); charIt.Advance(1))
    {
      char32 c = text::CodePointAt(charIt.Text(), 0);
      if ((!DeprecatedIsSpaceOrNewline(c) && c != 0xA0 /* noBreakSpace */) || c == '\n')
      {
        break;
      }
      _end = dom::MakeDeprecatedLegacyPosition(charIt.Range().End);
      if (_anchorIsFirst)
      {
        _focus = _end;
      }
      else
      {
        _anchor = _end;
      }
    }
  }

  void VisibleSelection::SetBaseAndExtentToDeepEquivalents() noexcept
  {
    // If only one of anchor and focus is null, convert to a caret selection.
    // FIXME: Seems like a better rule would be to convert to no selection.
    if (_anchor.IsNull())
    {
      _anchor = _focus;
    }
    if (_focus.IsNull())
    {
      _focus = _anchor;
    }

    _anchorIsFirst = std::is_lteq(dom::TreeOrder<dom::TreeType::ShadowIncludingTree>(_anchor, _focus));

    _base = VisiblePosition(_anchor, _affinity).DeepEquivalent();
    if (_anchor == _focus)
    {
      _extent = _base;
    }
    else
    {
      _extent = VisiblePosition(_focus, _affinity).DeepEquivalent();
    }
    if (_base.IsNull() != _extent.IsNull())
    {
      if (_base.IsNull())
      {
        _base = _extent;
      }
      else
      {
        _extent = _base;
      }
    }
  }

  void VisibleSelection::AdjustSelectionRespectingGranularity(TextGranularity granularity) noexcept
  {
    switch (granularity)
    {
      case TextGranularity::CharacterGranularity:
        // Don't do any expansion.
        break;
      case TextGranularity::WordGranularity:
      {
        // General case: Select the word the caret is positioned inside of, or at the start of
        // (RightWordIfOnBoundary).
        // Edge case: If the caret is after the last word in a soft-wrapped line or the last word in
        // the document, select that last word (LeftWordIfOnBoundary).
        // Edge case: If the caret is after the last word in a paragraph, select from the end of the
        // last word to the line break (also RightWordIfOnBoundary);
        VisiblePosition start = VisiblePosition(_start, _affinity);
        VisiblePosition originalEnd(_end, _affinity);
        WordSide side = WordSide::RightWordIfOnBoundary;
        if (IsEndOfEditableOrNonEditableContent(start)
            || (IsEndOfLine(start) && !IsStartOfLine(start) && !IsEndOfParagraph(start)))
        {
          side = WordSide::LeftWordIfOnBoundary;
        }
        _start = StartOfWord(start, side).DeepEquivalent();
        side = WordSide::RightWordIfOnBoundary;
        if (IsEndOfEditableOrNonEditableContent(originalEnd)
            || (IsEndOfLine(originalEnd) && !IsStartOfLine(originalEnd) && !IsEndOfParagraph(originalEnd)))
        {
          side = WordSide::LeftWordIfOnBoundary;
        }

        VisiblePosition wordEnd(EndOfWord(originalEnd, side));
        VisiblePosition end(wordEnd);

        if (IsEndOfParagraph(originalEnd) && !IsEmptyTableCell(_start.DeprecatedNode()))
        {
          // Select the paragraph break (the space from the end of a paragraph to the start of
          // the next one) to match TextEdit.
          end = wordEnd.Next();

          if (RefPtr table = IsFirstPositionAfterTable(end))
          {
            // The paragraph break after the last paragraph in the last cell of a block table ends
            // at the start of the paragraph after the table.
            if (IsBlock(*table))
            {
              end = end.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
            }
            else
            {
              end = wordEnd;
            }
          }

          if (end.IsNull())
          {
            end = wordEnd;
          }
        }

        _end = end.DeepEquivalent();

        // End must not be before start.
        if (_start.DeprecatedNode() == _end.DeprecatedNode()
            && _start.DeprecatedEditingOffset() > _end.DeprecatedEditingOffset())
        {
          std::swap(_start, _end);
        }
        break;
      }
      case TextGranularity::SentenceGranularity:
        _start = StartOfSentence(VisiblePosition(_start, _affinity)).DeepEquivalent();
        _end = EndOfSentence(VisiblePosition(_end, _affinity)).DeepEquivalent();
        break;
      case TextGranularity::LineGranularity:
      {
        _start = StartOfLine(VisiblePosition(_start, _affinity)).DeepEquivalent();
        VisiblePosition end = EndOfLine(VisiblePosition(_end, _affinity));
        // If the end of this line is at the end of a paragraph, include the space
        // after the end of the line in the selection.
        if (IsEndOfParagraph(end))
        {
          VisiblePosition next = end.Next();
          if (next.IsNotNull())
          {
            end = next;
          }
        }
        _end = end.DeepEquivalent();
        break;
      }
      case TextGranularity::LineBoundary:
        _start = StartOfLine(VisiblePosition(_start, _affinity)).DeepEquivalent();
        _end = EndOfLine(VisiblePosition(_end, _affinity)).DeepEquivalent();
        break;
      case TextGranularity::ParagraphGranularity:
      {
        VisiblePosition position(_start, _affinity);
        if (IsStartOfLine(position) && IsEndOfEditableOrNonEditableContent(position))
        {
          position = position.Previous();
        }
        _start = StartOfParagraph(position).DeepEquivalent();
        VisiblePosition visibleParagraphEnd = EndOfParagraph(VisiblePosition(_end, _affinity));

        // Include the "paragraph break" (the space from the end of this paragraph to the start
        // of the next one) in the selection.
        VisiblePosition end(visibleParagraphEnd.Next());

        if (RefPtr table = IsFirstPositionAfterTable(end))
        {
          // The paragraph break after the last paragraph in the last cell of a block table ends
          // at the start of the paragraph after the table, not at the position just after the table.
          if (IsBlock(*table))
          {
            end = end.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
          }
          // There is no parargraph break after the last paragraph in the last cell of an inline table.
          else
          {
            end = visibleParagraphEnd;
          }
        }

        if (end.IsNull())
        {
          end = visibleParagraphEnd;
        }

        _end = end.DeepEquivalent();
        break;
      }
      case TextGranularity::DocumentBoundary:
        _start = StartOfDocument(_start.GetDocument()).DeepEquivalent();
        _end = EndOfDocument(_end.GetDocument()).DeepEquivalent();
        break;
      case TextGranularity::ParagraphBoundary:
        _start = StartOfParagraph(VisiblePosition(_start, _affinity)).DeepEquivalent();
        _end = EndOfParagraph(VisiblePosition(_end, _affinity)).DeepEquivalent();
        break;
      case TextGranularity::SentenceBoundary:
        _start = StartOfSentence(VisiblePosition(_start, _affinity)).DeepEquivalent();
        _end = EndOfSentence(VisiblePosition(_end, _affinity)).DeepEquivalent();
        break;
      case TextGranularity::DocumentGranularity: krys_debug_assert(false); break;
    }

    // Make sure we do not have a dangling start or end.
    if (_start.IsNull())
    {
      _start = _end;
    }
    if (_end.IsNull())
    {
      _end = _start;
    }
  }

  void VisibleSelection::UpdateSelectionType() noexcept
  {
    if (_start.IsNull())
    {
      krys_debug_assert(_end.IsNull());
      _type = Type::None;
      _affinity = Affinity::Downstream;
    }
    else if (_start == _end || _start.Upstream() == _end.Upstream())
    {
      _type = Type::Caret;
    }
    else
    {
      _type = Type::Range;
      _affinity = Affinity::Downstream;
    }
  }

  void VisibleSelection::Validate(TextGranularity granularity) noexcept
  {
    SetBaseAndExtentToDeepEquivalents();

    _start = _anchorIsFirst ? _base : _extent;
    _end = _anchorIsFirst ? _extent : _base;

    auto startBeforeAdjustments = _start;
    auto endBeforeAdjustments = _end;

    AdjustSelectionRespectingGranularity(granularity);
    AdjustSelectionToAvoidCrossingShadowBoundaries();
    AdjustSelectionToAvoidCrossingEditingBoundaries();
    UpdateSelectionType();

    bool shouldUpdateAnchor = _start != startBeforeAdjustments;
    bool shouldUpdateFocus = _end != endBeforeAdjustments;

    if (IsRange())
    {
      // "Constrain" the selection to be the smallest equivalent range of nodes.
      // This is a somewhat arbitrary choice, but experience shows that it is
      // useful to make to make the selection "canonical" (if only for
      // purposes of comparing selections). This is an ideal point of the code
      // to do this operation, since all selection changes that result in a RANGE
      // come through here before anyone uses it.
      // FIXME: Canonicalizing is good, but haven't we already done it (when we
      // set these two positions to VisiblePosition DeepEquivalent()s above)?
      _start = _start.Downstream();
      _end = _end.Upstream();

      // Position::Downstream() or Position::Upstream() might violate editing boundaries
      // if an anchor node has a Shadow DOM even though they should not. But because this
      // happens in practice, adjust selection to avoid crossing editing boundaries again.
      AdjustSelectionToAvoidCrossingEditingBoundaries();
    }

    if (shouldUpdateAnchor)
    {
      _anchor = _anchorIsFirst ? _start : _end;
      _base = _anchor;
    }
    if (shouldUpdateFocus)
    {
      _focus = _anchorIsFirst ? _end : _start;
      _extent = _focus;
    }
  }

  // Because we use VisibleSelection to store values in editing commands for use when
  // undoing the command, we need to be able to create a selection that, while currently
  // invalid, will be valid once the changes are undone. This is a design problem.
  // The best fix is likely to get rid of canonicalization from VisibleSelection entirely,
  // and then remove this function.
  void VisibleSelection::SetWithoutValidation(const dom::Position &anchor,
                                              const dom::Position &focus) noexcept
  {
    krys_debug_assert(anchor.IsNull() == focus.IsNull());
    krys_debug_assert(_affinity == Affinity::Downstream);
    _anchor = anchor;
    _focus = focus;
    _anchorIsFirst = std::is_lteq(dom::TreeOrder<dom::TreeType::ShadowIncludingTree>(_anchor, _focus));
    _base = anchor;
    _extent = focus;
    _start = _anchorIsFirst ? anchor : focus;
    _end = _anchorIsFirst ? focus : anchor;
    _type = anchor == focus ? Type::Caret : Type::Range;
  }

  dom::Position VisibleSelection::AdjustPositionForEnd(const dom::Position &currentPosition,
                                                       dom::Node *startContainerNode) noexcept
  {
    auto &scopeRoot = dom::TreeQueries::Root(*startContainerNode);

    krys_debug_assert(&dom::TreeQueries::Root(*currentPosition.GetContainerNode()) != &scopeRoot);

    if (auto *ancestor = AncestorNodeInThisScope(scopeRoot, currentPosition.GetContainerNode()))
    {
      if (ancestor->Contains(startContainerNode))
      {
        return dom::PositionAfterNode(ancestor);
      }
      return dom::PositionBeforeNode(ancestor);
    }

    if (auto *lastChild = scopeRoot.LastChild())
    {
      return dom::PositionAfterNode(lastChild);
    }

    return {};
  }

  dom::Position VisibleSelection::AdjustPositionForStart(const dom::Position &currentPosition,
                                                         dom::Node *endContainerNode) noexcept
  {
    auto &scopeRoot = dom::TreeQueries::Root(*endContainerNode);

    krys_debug_assert(&dom::TreeQueries::Root(*currentPosition.GetContainerNode()) != &scopeRoot);

    if (auto *ancestor = AncestorNodeInThisScope(scopeRoot, currentPosition.GetContainerNode()))
    {
      if (ancestor->Contains(endContainerNode))
      {
        return dom::PositionBeforeNode(ancestor);
      }
      return dom::PositionAfterNode(ancestor);
    }

    if (auto *firstChild = scopeRoot.FirstChild())
    {
      return dom::PositionBeforeNode(firstChild);
    }

    return {};
  }

  void VisibleSelection::AdjustSelectionToAvoidCrossingShadowBoundaries() noexcept
  {
    if (_start.IsNull() || _end.IsNull())
    {
      return;
    }

    auto &startNode = *_start.AnchorNode();
    auto &endNode = *_end.AnchorNode();
    if (&dom::TreeQueries::Root(startNode) == &dom::TreeQueries::Root(endNode))
    {
      return;
    }

    if (!IsInUserAgentShadowRootOrHasEditableShadowAncestor(startNode)
        && !IsInUserAgentShadowRootOrHasEditableShadowAncestor(endNode))
    {
      return;
    }

    // Correct the focus if necessary.
    if (_anchorIsFirst)
    {
      _extent = AdjustPositionForEnd(_end, _start.GetContainerNode());
      _end = _extent;
    }
    else
    {
      _extent = AdjustPositionForStart(_start, _end.GetContainerNode());
      _start = _extent;
    }
    _focus = _extent;
  }

  void VisibleSelection::AdjustSelectionToAvoidCrossingEditingBoundaries() noexcept
  {
    if (_start.IsNull() || _end.IsNull())
    {
      return;
    }

    // Early return in the caret case (the state hasn't actually been set yet, so we can't use IsCaret()) to
    // avoid the expense of computing HighestEditableRoot.
    if (_base == _start && _base == _end)
    {
      return;
    }

    auto baseRoot = HighestEditableRoot(_base);
    auto startRoot = HighestEditableRoot(_start);
    auto endRoot = HighestEditableRoot(_end);

    auto *baseEditableAncestor = LowestEditableAncestor(_base.GetContainerNode());

    // The base, start and end are all in the same region.  No adjustment necessary.
    if (baseRoot == startRoot && baseRoot == endRoot)
    {
      return;
    }

    // The selection is based in editable content.
    if (baseRoot)
    {
      // If the start is outside the base's editable root, cap it at the start of that root.
      // If the start is in non-editable content that is inside the base's editable root, put it
      // at the first editable position after start inside the base's editable root.
      if (startRoot != baseRoot)
      {
        VisiblePosition first {FirstEditablePositionAfterPositionInRoot(_start, baseRoot.get())};
        _start = first.DeepEquivalent();
        if (_start.IsNull())
        {
          krys_debug_assert(false);
          _start = _end;
        }
      }
      // If the end is outside the base's editable root, cap it at the end of that root.
      // If the end is in non-editable content that is inside the base's root, put it
      // at the last editable position before the end inside the base's root.
      if (endRoot != baseRoot)
      {
        VisiblePosition last {LastEditablePositionBeforePositionInRoot(_end, baseRoot.get())};
        _end = last.DeepEquivalent();
        if (_end.IsNull())
        {
          _end = _start;
        }
      }
    }
    // The selection is based in non-editable content.
    else
    {
      // FIXME: Non-editable pieces inside editable content should be atomic, in the same way that editable
      // pieces in non-editable content are atomic.

      // The selection ends in editable content or non-editable content inside a different editable ancestor,
      // move backward until non-editable content inside the same lowest editable ancestor is reached.
      auto *endEditableAncestor = LowestEditableAncestor(_end.GetContainerNode());
      if (endRoot || endEditableAncestor != baseEditableAncestor)
      {
        dom::Position p = PreviousVisuallyDistinctCandidate(_end);
        dom::Element *shadowAncestor = endRoot ? dom::TreeQueries::ShadowHost(*endRoot) : nullptr;
        if (p.IsNull() && shadowAncestor)
        {
          p = dom::PositionAfterNode(shadowAncestor);
        }
        while (p.IsNotNull()
               && !(LowestEditableAncestor(p.GetContainerNode()) == baseEditableAncestor
                    && !IsEditablePosition(p)))
        {
          auto *root = EditableRootForPosition(p);
          shadowAncestor = root ? dom::TreeQueries::ShadowHost(*root) : nullptr;
          p = IsAtomicNode(p.GetContainerNode()) ? dom::PositionInParentBeforeNode(p.GetContainerNode())
                                                 : PreviousVisuallyDistinctCandidate(p);
          if (p.IsNull() && shadowAncestor)
          {
            p = dom::PositionAfterNode(shadowAncestor);
          }
        }
        VisiblePosition previous(p);

        if (previous.IsNull())
        {
          *this = {};
          return;
        }
        _end = previous.DeepEquivalent();
      }

      // The selection starts in editable content or non-editable content inside a different editable
      // ancestor, move forward until non-editable content inside the same lowest editable ancestor is
      // reached.
      auto *startEditableAncestor = LowestEditableAncestor(_start.GetContainerNode());
      if (startRoot || startEditableAncestor != baseEditableAncestor)
      {
        dom::Position p = NextVisuallyDistinctCandidate(_start);
        dom::Element *shadowAncestor = startRoot ? dom::TreeQueries::ShadowHost(*startRoot) : nullptr;
        if (p.IsNull() && shadowAncestor)
        {
          p = dom::PositionBeforeNode(shadowAncestor);
        }
        while (p.IsNotNull()
               && !(LowestEditableAncestor(p.GetContainerNode()) == baseEditableAncestor
                    && !IsEditablePosition(p)))
        {
          auto *root = EditableRootForPosition(p);
          shadowAncestor = root ? dom::TreeQueries::ShadowHost(*root) : nullptr;
          p = IsAtomicNode(p.GetContainerNode()) ? dom::PositionInParentAfterNode(p.GetContainerNode())
                                                 : NextVisuallyDistinctCandidate(p);
          if (p.IsNull() && shadowAncestor)
          {
            p = dom::PositionBeforeNode(shadowAncestor);
          }
        }
        VisiblePosition next(p);

        if (next.IsNull())
        {
          *this = {};
          return;
        }
        _start = next.DeepEquivalent();
      }
    }

    // Correct the focus if necessary.
    if (baseEditableAncestor != LowestEditableAncestor(_extent.GetContainerNode()))
    {
      _extent = _anchorIsFirst ? _end : _start;
      _focus = _extent;
    }
  }

  bool VisibleSelection::IsContentEditable() const noexcept
  {
    return IsEditablePosition(Start());
  }

  bool VisibleSelection::HasEditableStyle() const noexcept
  {
    if (auto *containerNode = Start().GetContainerNode())
    {
      return containerNode->HasEditableStyle();
    }
    return false;
  }

  bool VisibleSelection::IsContentRichlyEditable() const noexcept
  {
    return IsRichlyEditablePosition(Start());
  }

  dom::Element *VisibleSelection::RootEditableElement() const noexcept
  {
    return EditableRootForPosition(Start());
  }

  bool VisibleSelection::IsInPasswordField() const noexcept
  {
    auto *textControl = DynamicDowncast<html::HTMLInputElement>(html::EnclosingTextFormControl(Start()));
    return textControl && textControl->IsPasswordField();
  }

  dom::Node *VisibleSelection::NonBoundaryShadowTreeRootNode() const noexcept
  {
    auto *node = Start().DeprecatedNode();
    return node && !Is<dom::ShadowRoot>(*node) ? node->NonBoundaryShadowTreeRootNode() : nullptr;
  }
}

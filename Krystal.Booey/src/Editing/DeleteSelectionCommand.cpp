#include "Krystal.Booey/Editing/DeleteSelectionCommand.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/EditingStyle.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

namespace krys::boo
{
  using dom::TreeQueries;

  namespace
  {
    KRYS_NODISCARD bool IsTableRow(const dom::Node &node) noexcept
    {
      return IsHTMLElementOfType(&node, html::TagName::tr);
    }

    KRYS_NODISCARD bool IsTableCellEmpty(dom::Node &cell) noexcept
    {
      krys_debug_assert(IsTableCell(cell));
      return VisiblePosition(dom::FirstPositionInNode(&cell))
             == VisiblePosition(dom::LastPositionInNode(&cell));
    }

    KRYS_NODISCARD bool IsTableRowEmpty(const dom::Node &row) noexcept
    {
      if (!IsTableRow(row))
      {
        return false;
      }

      for (auto *child = row.FirstChild(); child; child = child->NextSibling())
      {
        if (IsTableCell(*child) && !IsTableCellEmpty(*child))
        {
          return false;
        }
      }

      return true;
    }

    KRYS_NODISCARD bool IsSpecialHTMLElement(const dom::Node &node) noexcept
    {
      auto *htmlElement = DynamicDowncast<html::HTMLElement>(node);
      if (!htmlElement)
      {
        return false;
      }

      if (css::IsLink(*htmlElement))
      {
        return true;
      }

      auto *renderer = htmlElement->Renderer();
      if (!renderer)
      {
        return false;
      }

      const auto &style = renderer->Style();
      if (style.EffectiveDisplay() == DisplayType::Table
          || style.EffectiveDisplay() == DisplayType::InlineTable)
      {
        return true;
      }

      if (style.IsFloating())
      {
        return true;
      }

      if (style.GetPosition() != PositionType::Static)
      {
        return true;
      }

      return false;
    }

    KRYS_NODISCARD RefPtr<html::HTMLElement> FirstInSpecialElement(const dom::Position &position) noexcept
    {
      auto *rootEditableElement = position.RootEditableElement();
      for (auto *node = position.DeprecatedNode(); node && node->RootEditableElement() == rootEditableElement;
           node = node->ParentNode())
      {
        if (!IsSpecialHTMLElement(*node))
        {
          continue;
        }
        VisiblePosition visiblePosition = position;
        VisiblePosition firstInElement = FirstPositionInOrBeforeNode(node);
        if ((IsRenderedTable(node) && visiblePosition == firstInElement.Next())
            || visiblePosition == firstInElement)
        {
          return ShareRefPtr(&Downcast<html::HTMLElement>(*node));
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD RefPtr<html::HTMLElement> LastInSpecialElement(const dom::Position &position) noexcept
    {
      auto *rootEditableElement = position.RootEditableElement();
      for (auto *node = position.DeprecatedNode(); node && node->RootEditableElement() == rootEditableElement;
           node = node->ParentNode())
      {
        if (!IsSpecialHTMLElement(*node))
        {
          continue;
        }
        VisiblePosition visiblePosition = position;
        VisiblePosition lastInElement = LastPositionInOrAfterNode(node);
        if ((IsRenderedTable(node) && visiblePosition == lastInElement.Previous())
            || visiblePosition == lastInElement)
        {
          return ShareRefPtr(&Downcast<html::HTMLElement>(*node));
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD std::pair<dom::Position, RefPtr<html::HTMLElement>>
      PositionBeforeContainingSpecialElement(const dom::Position &position) noexcept
    {
      auto element = FirstInSpecialElement(position);
      if (!element)
      {
        return {position, nullptr};
      }
      auto result = dom::PositionInParentBeforeNode(element.get());
      if (result.IsNull()
          || result.GetContainerNode()->RootEditableElement()
               != position.GetContainerNode()->RootEditableElement())
      {
        return {position, nullptr};
      }
      return {result, krys::move(element)};
    }

    KRYS_NODISCARD std::pair<dom::Position, RefPtr<html::HTMLElement>>
      PositionAfterContainingSpecialElement(const dom::Position &position) noexcept
    {
      auto element = LastInSpecialElement(position);
      if (!element)
      {
        return {position, nullptr};
      }
      auto result = dom::PositionInParentAfterNode(element.get());
      if (result.IsNull()
          || result.DeprecatedNode()->RootEditableElement()
               != position.GetContainerNode()->RootEditableElement())
      {
        return {position, nullptr};
      }
      return {result, krys::move(element)};
    }

    KRYS_NODISCARD dom::Position FirstEditablePositionInNode(dom::Node *node) noexcept
    {
      krys_debug_assert(node);
      auto *next = node;
      while (next && !next->HasEditableStyle())
      {
        next = TreeQueries::Next(*next, node);
      }
      return next ? FirstPositionInOrBeforeNode(next) : dom::Position();
    }

    KRYS_NODISCARD bool ShouldRemoveContentOnly(const dom::Node &node) noexcept
    {
      return IsTableStructureNode(node) || node.IsRootEditableElement();
    }

    void UpdatePositionForTextRemoval(dom::Node *node, int32 offset, int32 count,
                                      dom::Position &position) noexcept
    {
      if (position.GetAnchorType() != dom::Position::AnchorType::PositionIsOffsetInAnchor
          || position.GetContainerNode() != node)
      {
        return;
      }

      if (position.OffsetInContainerNode() > offset + count)
      {
        position.MoveToOffset(static_cast<uint32>(position.OffsetInContainerNode() - count));
      }
      else if (position.OffsetInContainerNode() > offset)
      {
        position.MoveToOffset(static_cast<uint32>(offset));
      }
    }

    KRYS_NODISCARD dom::Element *NextElement(dom::Node &current, const dom::Node *stayWithin) noexcept
    {
      for (auto *node = TreeQueries::Next(current, stayWithin); node;
           node = TreeQueries::Next(*node, stayWithin))
      {
        if (auto *element = DynamicDowncast<dom::Element>(*node))
        {
          return element;
        }
      }
      return nullptr;
    }
  }

  DeleteSelectionCommand::DeleteSelectionCommand(dom::Document &document, bool smartDelete,
                                                 bool mergeBlocksAfterDelete, bool replace,
                                                 bool expandForSpecialElements, bool sanitizeMarkup,
                                                 EditAction editingAction) noexcept
      : CompositeEditCommand(document, editingAction), _hasSelectionToDelete(false),
        _smartDelete(smartDelete), _mergeBlocksAfterDelete(mergeBlocksAfterDelete), _replace(replace),
        _expandForSpecialElements(expandForSpecialElements), _sanitizeMarkup(sanitizeMarkup)
  {
  }

  DeleteSelectionCommand::DeleteSelectionCommand(const VisibleSelection &selection, bool smartDelete,
                                                 bool mergeBlocksAfterDelete, bool replace,
                                                 bool expandForSpecialElements, bool sanitizeMarkup,
                                                 EditAction editingAction) noexcept
      : CompositeEditCommand(selection.Start().AnchorNode()->NodeDocument(), editingAction),
        _hasSelectionToDelete(true), _smartDelete(smartDelete),
        _mergeBlocksAfterDelete(mergeBlocksAfterDelete), _replace(replace),
        _expandForSpecialElements(expandForSpecialElements), _sanitizeMarkup(sanitizeMarkup),
        _selectionToDelete(selection)
  {
  }

  DeleteSelectionCommand::~DeleteSelectionCommand() noexcept = default;

  Ref<DeleteSelectionCommand> DeleteSelectionCommand::Create(dom::Document &document, bool smartDelete,
                                                             bool mergeBlocksAfterDelete, bool replace,
                                                             bool expandForSpecialElements,
                                                             bool sanitizeMarkup,
                                                             EditAction editingAction) noexcept
  {
    return CreateRef<DeleteSelectionCommand>(document, smartDelete, mergeBlocksAfterDelete, replace,
                                             expandForSpecialElements, sanitizeMarkup, editingAction);
  }

  Ref<DeleteSelectionCommand> DeleteSelectionCommand::Create(const VisibleSelection &selection,
                                                             bool smartDelete, bool mergeBlocksAfterDelete,
                                                             bool replace, bool expandForSpecialElements,
                                                             bool sanitizeMarkup,
                                                             EditAction editingAction) noexcept
  {
    return CreateRef<DeleteSelectionCommand>(selection, smartDelete, mergeBlocksAfterDelete, replace,
                                             expandForSpecialElements, sanitizeMarkup, editingAction);
  }

  void DeleteSelectionCommand::InitializeStartEnd(dom::Position &start, dom::Position &end) noexcept
  {
    start = _selectionToDelete.Start();
    end = _selectionToDelete.End();

    // For HRs, we'll get a position at (HR,1) when hitting delete from the beginning of the previous line, or
    // (HR,0) when forward deleting, but in these cases, we want to delete it, so manually expand the
    // selection
    if (IsHTMLElementOfType(start.DeprecatedNode(), html::TagName::hr))
    {
      start = dom::PositionBeforeNode(start.DeprecatedNode());
    }
    else if (IsHTMLElementOfType(end.DeprecatedNode(), html::TagName::hr))
    {
      end = dom::PositionAfterNode(end.DeprecatedNode());
    }

    // FIXME: This is only used so that moveParagraphs can avoid the bugs in special element expansion.
    if (!_expandForSpecialElements)
    {
      return;
    }

    while (true)
    {
      auto [startBeforeSpecialElement, startSpecialContainer] = PositionBeforeContainingSpecialElement(start);
      auto [endAfterSpecialElement, endSpecialContainer] = PositionAfterContainingSpecialElement(end);

      if (!startSpecialContainer && !endSpecialContainer)
      {
        break;
      }

      _mergeBlocksAfterDelete = false;

      if (VisiblePosition(start) != _selectionToDelete.VisibleStart()
          || VisiblePosition(end) != _selectionToDelete.VisibleEnd())
      {
        break;
      }

      // If we're going to expand to include the startSpecialContainer, it must be fully selected.
      if (startSpecialContainer && !endSpecialContainer
          && dom::PositionInParentAfterNode(startSpecialContainer.get()) >= end)
      {
        break;
      }

      // If we're going to expand to include the endSpecialContainer, it must be fully selected.
      if (endSpecialContainer && !startSpecialContainer
          && start >= dom::PositionInParentBeforeNode(endSpecialContainer.get()))
      {
        break;
      }

      if (startSpecialContainer && startSpecialContainer->IsDescendantOf(endSpecialContainer.get()))
      {
        // Don't adjust the end yet, it is the end of a special element that contains the start
        // special element (which may or may not be fully selected).
        start = startBeforeSpecialElement;
      }
      else if (endSpecialContainer && endSpecialContainer->IsDescendantOf(startSpecialContainer.get()))
      {
        // Don't adjust the start yet, it is the start of a special element that contains the end
        // special element (which may or may not be fully selected).
        end = endAfterSpecialElement;
      }
      else
      {
        start = startBeforeSpecialElement;
        end = endAfterSpecialElement;
      }
    }
  }

  void DeleteSelectionCommand::SetStartingSelectionOnSmartDelete(const dom::Position &start,
                                                                 const dom::Position &end) noexcept
  {
    VisiblePosition newBase;
    VisiblePosition newExtent;
    if (StartingSelection().IsBaseFirst())
    {
      newBase = start;
      newExtent = end;
    }
    else
    {
      newBase = end;
      newExtent = start;
    }
    SetStartingSelection(VisibleSelection(newBase, newExtent, StartingSelection().GetDirectionality()));
  }

  bool DeleteSelectionCommand::ShouldSmartDeleteParagraphSpacers() noexcept
  {
    return GetDocument().GetEditor().Behavior().ShouldSmartInsertDeleteParagraphs();
  }

  void DeleteSelectionCommand::SmartDeleteParagraphSpacers() noexcept
  {
    VisiblePosition visibleStart {_upstreamStart};
    VisiblePosition visibleEnd {_downstreamEnd};
    bool selectionEndsInParagraphSeparator = IsEndOfParagraph(visibleEnd);
    bool selectionEndIsEndOfContent = EndOfEditableContent(visibleEnd) == visibleEnd;
    bool startAndEndInSameUnsplittableElement =
      UnsplittableElementForPosition(visibleStart.DeepEquivalent())
      == UnsplittableElementForPosition(visibleEnd.DeepEquivalent());
    visibleStart = visibleStart.Previous(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
    visibleEnd = visibleEnd.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
    bool previousPositionIsStartOfContent = StartOfEditableContent(visibleStart) == visibleStart;
    bool previousPositionIsBlankParagraph = IsBlankParagraph(visibleStart);
    bool endPositionIsBlankParagraph = IsBlankParagraph(visibleEnd);
    bool hasBlankParagraphAfterEndOrIsEndOfContent =
      !selectionEndIsEndOfContent && (endPositionIsBlankParagraph || selectionEndsInParagraphSeparator);
    if (startAndEndInSameUnsplittableElement && previousPositionIsBlankParagraph
        && hasBlankParagraphAfterEndOrIsEndOfContent)
    {
      _needPlaceholder = false;
      dom::Position position;
      if (endPositionIsBlankParagraph)
      {
        position = StartOfNextParagraph(StartOfNextParagraph(_downstreamEnd)).DeepEquivalent();
      }
      else
      {
        position = VisiblePosition(_downstreamEnd).Next().DeepEquivalent();
      }
      _upstreamEnd = position.Upstream();
      _downstreamEnd = position.Downstream();
      _trailingWhitespace = _downstreamEnd.TrailingWhitespacePosition(VisiblePosition::DefaultAffinity);
      SetStartingSelectionOnSmartDelete(_upstreamStart, _downstreamEnd);
    }
    if (startAndEndInSameUnsplittableElement && selectionEndIsEndOfContent && previousPositionIsBlankParagraph
        && selectionEndsInParagraphSeparator)
    {
      _needPlaceholder = false;
      VisiblePosition endOfParagraphBeforeStart;
      if (previousPositionIsStartOfContent)
      {
        endOfParagraphBeforeStart = EndOfParagraph(VisiblePosition {_upstreamStart}.Previous());
      }
      else
      {
        endOfParagraphBeforeStart = EndOfParagraph(VisiblePosition {_upstreamStart}.Previous().Previous());
      }
      auto position = endOfParagraphBeforeStart.DeepEquivalent();
      _upstreamStart = position.Upstream();
      _downstreamStart = position.Downstream();
      _leadingWhitespace = _upstreamStart.LeadingWhitespacePosition(Affinity::Downstream);
      SetStartingSelectionOnSmartDelete(_upstreamStart, _upstreamEnd);
    }
  }

  bool DeleteSelectionCommand::InitializePositionData() noexcept
  {
    dom::Position start, end;
    InitializeStartEnd(start, end);

    if (!IsEditablePosition(start, EditableType::ContentIsEditable))
    {
      start = FirstEditablePositionAfterPositionInRoot(start, HighestEditableRoot(start).get());
    }
    if (!IsEditablePosition(end, EditableType::ContentIsEditable))
    {
      end = LastEditablePositionBeforePositionInRoot(end, HighestEditableRoot(start).get());
    }

    if (start.IsNull() || end.IsNull())
    {
      return false;
    }

    _upstreamStart = start.Upstream();
    _downstreamStart = start.Downstream();
    _upstreamEnd = end.Upstream();
    _downstreamEnd = end.Downstream();

    _startRoot = ShareRefPtr<dom::Node>(EditableRootForPosition(start));
    _endRoot = ShareRefPtr<dom::Node>(EditableRootForPosition(end));

    _startTableRow = EnclosingNodeOfType(start, &IsTableRow);
    _endTableRow = EnclosingNodeOfType(end, &IsTableRow);

    // Don't move content out of a table cell.
    // If the cell is non-editable, enclosingNodeOfType won't return it by default, so
    // tell that function that we don't care if it returns non-editable nodes.
    auto startCell =
      EnclosingNodeOfType(_upstreamStart, &IsTableCell, EditingBoundaryCrossingRule::CanCrossEditingBoundary);
    auto endCell =
      EnclosingNodeOfType(_downstreamEnd, &IsTableCell, EditingBoundaryCrossingRule::CanCrossEditingBoundary);
    // FIXME: This isn't right.  A borderless table with two rows and a single column would appear as two
    // paragraphs.
    if (endCell && endCell != startCell)
    {
      _mergeBlocksAfterDelete = false;
    }

    // Usually the start and the end of the selection to delete are pulled together as a result of the
    // deletion. Sometimes they aren't (like when no merge is requested), so we must choose one position to
    // hold the caret and receive the placeholder after deletion.
    VisiblePosition visibleEnd(_downstreamEnd);
    if (_mergeBlocksAfterDelete && !IsEndOfParagraph(visibleEnd))
    {
      _endingPosition = _downstreamEnd;
    }
    else
    {
      _endingPosition = _downstreamStart;
    }

    // We don't want to merge into a block if it will mean changing the quote level of content after deleting
    // selections that contain a whole number paragraphs plus a line break, since it is unclear to most users
    // that such a selection actually ends at the start of the next paragraph. This matches TextEdit behavior
    // for indented paragraphs.
    // Only apply this rule if the endingSelection is a range selection.  If it is a caret, then other
    // operations have created the selection we're deleting (like the process of creating a selection to
    // delete during a backspace), and the user isn't in the situation described above.
    if (NumEnclosingMailBlockquotes(start) != NumEnclosingMailBlockquotes(end)
        && IsStartOfParagraph(visibleEnd) && IsStartOfParagraph(VisiblePosition(start))
        && EndingSelection().IsRange())
    {
      _mergeBlocksAfterDelete = false;
      _pruneStartBlockIfNecessary = true;
    }

    // Handle leading and trailing whitespace, as well as smart delete adjustments to the selection
    _leadingWhitespace = _upstreamStart.LeadingWhitespacePosition(_selectionToDelete.GetAffinity());
    _trailingWhitespace = _downstreamEnd.TrailingWhitespacePosition(VisiblePosition::DefaultAffinity);

    if (_smartDelete)
    {
      // skip smart delete if the selection to delete already starts or ends with whitespace
      dom::Position pos = VisiblePosition(_upstreamStart, _selectionToDelete.GetAffinity()).DeepEquivalent();
      bool skipSmartDelete =
        IsEditablePosition(pos)
        && pos.TrailingWhitespacePosition(VisiblePosition::DefaultAffinity, true).IsNotNull();
      if (!skipSmartDelete)
      {
        skipSmartDelete =
          _downstreamEnd.LeadingWhitespacePosition(VisiblePosition::DefaultAffinity, true).IsNotNull();
      }

      // extend selection upstream if there is whitespace there
      bool hasLeadingWhitespaceBeforeAdjustment =
        _upstreamStart.LeadingWhitespacePosition(_selectionToDelete.GetAffinity(), true).IsNotNull();
      if (!skipSmartDelete && hasLeadingWhitespaceBeforeAdjustment)
      {
        auto visiblePos = VisiblePosition(_upstreamStart).Previous();
        pos = visiblePos.DeepEquivalent();
        // Expand out one character upstream for smart delete and recalculate
        // positions based on this change.
        _upstreamStart = pos.Upstream();
        _downstreamStart = pos.Downstream();
        _leadingWhitespace = _upstreamStart.LeadingWhitespacePosition(visiblePos.GetAffinity());

        SetStartingSelectionOnSmartDelete(_upstreamStart, _upstreamEnd);
      }

      // trailing whitespace is only considered for smart delete if there is no leading
      // whitespace, as in the case where you double-click the first word of a paragraph.
      if (!skipSmartDelete && !hasLeadingWhitespaceBeforeAdjustment
          && _downstreamEnd.TrailingWhitespacePosition(VisiblePosition::DefaultAffinity, true).IsNotNull())
      {
        // Expand out one character downstream for smart delete and recalculate
        // positions based on this change.
        pos = VisiblePosition(_downstreamEnd).Next().DeepEquivalent();
        _upstreamEnd = pos.Upstream();
        _downstreamEnd = pos.Downstream();
        _trailingWhitespace = _downstreamEnd.TrailingWhitespacePosition(VisiblePosition::DefaultAffinity);

        SetStartingSelectionOnSmartDelete(_downstreamStart, _downstreamEnd);
      }

      if (ShouldSmartDeleteParagraphSpacers())
      {
        SmartDeleteParagraphSpacers();
      }
    }

    // We must pass call parentAnchoredEquivalent on the positions since some editing positions
    // that appear inside their nodes aren't really inside them.  [hr, 0] is one example.
    // FIXME: parentAnchoredEquivalent should eventually be moved into enclosing element getters
    // like the one below, since editing functions should obviously accept editing positions.
    // FIXME: Passing false to enclosingNodeOfType tells it that it's OK to return a non-editable
    // node.  This was done to match existing behavior, but it seems wrong.
    _startBlock = EnclosingNodeOfType(_downstreamStart.ParentAnchoredEquivalent(), &IsBlock,
                                      EditingBoundaryCrossingRule::CanCrossEditingBoundary);
    _endBlock = EnclosingNodeOfType(_upstreamEnd.ParentAnchoredEquivalent(), &IsBlock,
                                    EditingBoundaryCrossingRule::CanCrossEditingBoundary);

    return true;
  }

  void DeleteSelectionCommand::SaveTypingStyleState() noexcept
  {
    // A common case is deleting characters that are all from the same text node. In
    // that case, the style at the start of the selection before deletion will be the
    // same as the style at the start of the selection after deletion (since those
    // two positions will be identical). Therefore there is no need to save the
    // typing style at the start of the selection, nor is there a reason to
    // compute the style at the start of the selection after deletion (see the
    // early return in calculateTypingStyleAfterDelete).
    // However, if typing style was previously set from another text node at the previous
    // position (now deleted), we need to clear that style as well.
    if (_upstreamStart.DeprecatedNode() == _downstreamEnd.DeprecatedNode()
        && Is<dom::Text>(*_upstreamStart.DeprecatedNode()))
    {
      GetDocument().Selection().ClearTypingStyle();
      return;
    }

    auto *startNode = _selectionToDelete.Start().DeprecatedNode();
    if (!Is<dom::Text>(*startNode) && !IsHTMLElementOfType(startNode, html::TagName::img)
        && !IsHTMLElementOfType(startNode, html::TagName::br))
    {
      return;
    }

    // Figure out the typing style in effect before the delete is done.
    _typingStyle = EditingStyle::Create(_selectionToDelete.Start(),
                                        EditingStyle::PropertiesToInclude::EditingPropertiesInEffect);
    _typingStyle->RemoveStyleAddedByNode(EnclosingAnchorElement(_selectionToDelete.Start()).get());

    // If we're deleting into a Mail blockquote, save the style at end() instead of start()
    // We'll use this later in computeTypingStyleAfterDelete if we end up outside of a Mail blockquote
    if (EnclosingNodeOfType(_selectionToDelete.Start(), IsMailBlockquote))
    {
      _deleteIntoBlockquoteStyle = EditingStyle::Create(_selectionToDelete.End());
    }
    else
    {
      _deleteIntoBlockquoteStyle = nullptr;
    }
  }

  bool DeleteSelectionCommand::HandleSpecialCaseBRDelete() noexcept
  {
    RefPtr<dom::Node> nodeAfterUpstreamStart = ShareRefPtr(_upstreamStart.ComputeNodeAfterPosition());
    RefPtr<dom::Node> nodeAfterDownstreamStart = ShareRefPtr(_downstreamStart.ComputeNodeAfterPosition());
    // Upstream end will appear before BR due to canonicalization
    RefPtr<dom::Node> nodeAfterUpstreamEnd = ShareRefPtr(_upstreamEnd.ComputeNodeAfterPosition());

    if (!nodeAfterUpstreamStart || !nodeAfterDownstreamStart)
    {
      return false;
    }

    // Check for special-case where the selection contains only a BR on a line by itself after another BR.
    bool upstreamStartIsBR = IsHTMLElementOfType(nodeAfterUpstreamStart.get(), html::TagName::br);
    bool downstreamStartIsBR = IsHTMLElementOfType(nodeAfterDownstreamStart.get(), html::TagName::br);
    // We should consider that the BR is on a line by itself also when we have <br><br>. This test should be
    // true only when the two elements are siblings and should be false in a case like <div><br></div><br>.
    bool isBROnLineByItself =
      upstreamStartIsBR && downstreamStartIsBR
      && ((nodeAfterDownstreamStart == nodeAfterUpstreamEnd)
          || (nodeAfterUpstreamEnd && IsHTMLElementOfType(nodeAfterUpstreamEnd.get(), html::TagName::br)
              && nodeAfterUpstreamEnd == nodeAfterUpstreamStart->NextSibling()));

    if (isBROnLineByItself)
    {
      RemoveNode(*nodeAfterDownstreamStart);
      return true;
    }

    // FIXME: This code doesn't belong in here.
    // We detect the case where the start is an empty line consisting of BR not wrapped in a block element.
    if (upstreamStartIsBR && downstreamStartIsBR
        && !(IsStartOfBlock(dom::PositionBeforeNode(nodeAfterUpstreamStart.get()))
             && IsEndOfBlock(dom::PositionAfterNode(nodeAfterDownstreamStart.get())))
        && (!nodeAfterUpstreamEnd || IsHTMLElementOfType(nodeAfterUpstreamEnd.get(), html::TagName::br)
            || nodeAfterUpstreamStart != nodeAfterUpstreamEnd->PreviousSibling()))
    {
      _startsAtEmptyLine = true;
      _endingPosition = _downstreamEnd;
    }

    return false;
  }

  void DeleteSelectionCommand::InsertBlockPlaceholderForTableCellIfNeeded(dom::Element &element) noexcept
  {
    // Make sure empty cell has some height.
    auto *renderer = DynamicDowncast<RenderTableCell>(element.Renderer());
    if (!renderer || renderer->ContentBoxHeight() > 0)
    {
      return;
    }
    (void)InsertBlockPlaceholder(FirstEditablePositionInNode(&element));
  }

  void DeleteSelectionCommand::RemoveNodeUpdatingStates(
    dom::Node &node, ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable) noexcept
  {
    if (&node == _startBlock)
    {
      auto prev = VisiblePosition(dom::FirstPositionInNode(_startBlock.get())).Previous();
      if (!prev.IsNull() && !IsEndOfBlock(prev))
      {
        _needPlaceholder = true;
      }
    }
    else if (&node == _endBlock)
    {
      auto next = VisiblePosition(dom::LastPositionInNode(_endBlock.get())).Next();
      if (!next.IsNull() && !IsStartOfBlock(next))
      {
        _needPlaceholder = true;
      }
    }

    // FIXME: Update the endpoints of the range being deleted.
    UpdatePositionForNodeRemoval(_endingPosition, node);
    UpdatePositionForNodeRemoval(_leadingWhitespace, node);
    UpdatePositionForNodeRemoval(_trailingWhitespace, node);

    CompositeEditCommand::RemoveNode(node, shouldAssumeContentIsAlwaysEditable);
  }

  void DeleteSelectionCommand::RemoveNode(
    dom::Node &node, ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable) noexcept
  {
    if (!node.ParentNode())
    {
      return;
    }

    Ref<dom::Node> protectedNode = ShareRef(node);
    if (_startRoot != _endRoot
        && !(node.IsDescendantOf(_startRoot.get()) && node.IsDescendantOf(_endRoot.get())))
    {
      // If a node is not in both the start and end editable roots, remove it only if its inside an editable
      // region.
      if (!node.ParentNode()->HasEditableStyle())
      {
        // Don't remove non-editable atomic nodes.
        if (!node.FirstChild())
        {
          return;
        }
        // Search this non-editable region for editable regions to empty.
        RefPtr<dom::Node> child = ShareRefPtr(node.FirstChild());
        while (child)
        {
          RefPtr<dom::Node> nextChild = ShareRefPtr(child->NextSibling());
          RemoveNode(*child, shouldAssumeContentIsAlwaysEditable);
          // Bail if nextChild is no longer node's child.
          if (nextChild && nextChild->ParentNode() != &node)
          {
            return;
          }
          child = krys::move(nextChild);
        }

        // Don't remove editable regions that are inside non-editable ones, just clear them.
        return;
      }
    }

    if (ShouldRemoveContentOnly(node))
    {
      // Do not remove an element of table structure; remove its contents.
      // Likewise for the root editable element.
      RefPtr<dom::Node> child = ShareRefPtr(TreeQueries::Next(node, &*protectedNode));
      while (child)
      {
        if (ShouldRemoveContentOnly(*child))
        {
          child = ShareRefPtr(TreeQueries::Next(*child, &*protectedNode));
          continue;
        }
        RefPtr<dom::Node> nextChild = ShareRefPtr(TreeQueries::NextSkippingChildren(*child, &*protectedNode));
        RemoveNodeUpdatingStates(*child, shouldAssumeContentIsAlwaysEditable);
        child = krys::move(nextChild);
      }

      krys_debug_assert(Is<dom::Element>(node));
      auto &element = Downcast<dom::Element>(node);
      (void)GetDocument().UpdateLayoutIgnorePendingStylesheets();
      // Check if we need to insert a placeholder for descendant table cells.
      RefPtr<dom::Element> descendant = ShareRefPtr(NextElement(element, &element));
      while (descendant)
      {
        RefPtr<dom::Element> nextDescendant = ShareRefPtr(NextElement(*descendant, &element));
        InsertBlockPlaceholderForTableCellIfNeeded(*descendant);
        descendant = krys::move(nextDescendant);
      }
      InsertBlockPlaceholderForTableCellIfNeeded(element);
      return;
    }
    RemoveNodeUpdatingStates(node, shouldAssumeContentIsAlwaysEditable);
  }

  void DeleteSelectionCommand::DeleteTextFromNode(dom::Text &node, uint32 offset, uint32 count) noexcept
  {
    // FIXME: Update the endpoints of the range being deleted.
    auto signedOffset = static_cast<int32>(offset);
    auto signedCount = static_cast<int32>(count);
    UpdatePositionForTextRemoval(&node, signedOffset, signedCount, _endingPosition);
    UpdatePositionForTextRemoval(&node, signedOffset, signedCount, _leadingWhitespace);
    UpdatePositionForTextRemoval(&node, signedOffset, signedCount, _trailingWhitespace);
    UpdatePositionForTextRemoval(&node, signedOffset, signedCount, _downstreamEnd);

    CompositeEditCommand::DeleteTextFromNode(node, offset, count);
  }

  void DeleteSelectionCommand::MakeStylingElementsDirectChildrenOfEditableRootToPreventStyleLoss() noexcept
  {
    auto range = _selectionToDelete.ToNormalizedRange();
    if (!range)
    {
      return;
    }
    auto nodes = dom::IntersectingNodes(*range).begin();
    List<Ref<dom::Element>> stylingElements;
    while (nodes)
    {
      auto &node = *nodes;
      auto shouldMove =
        IsHTMLElementOfType(&node, html::TagName::link) || IsHTMLElementOfType(&node, html::TagName::style);
      if (shouldMove)
      {
        nodes.AdvanceSkippingChildren();
        stylingElements.push_back(ShareRef(Downcast<dom::Element>(node)));
      }
      else
      {
        nodes.Advance();
      }
    }
    for (auto &stylingElement : stylingElements)
    {
      RefPtr<dom::Element> rootEditableElement = ShareRefPtr(stylingElement->RootEditableElement());
      if (!rootEditableElement)
      {
        continue;
      }
      RemoveNode(*stylingElement);
      AppendNode(*stylingElement, *rootEditableElement);
    }
  }

  void DeleteSelectionCommand::HandleGeneralDelete() noexcept
  {
    if (_upstreamStart.IsNull())
    {
      return;
    }

    int32 startOffset = _upstreamStart.DeprecatedEditingOffset();
    RefPtr<dom::Node> startNode = ShareRefPtr(_upstreamStart.DeprecatedNode());

    MakeStylingElementsDirectChildrenOfEditableRootToPreventStyleLoss();

    // Never remove the start block unless it's a table, in which case we won't merge content in.
    if (startNode == _startBlock && !startOffset && CanHaveChildrenForEditing(*startNode)
        && !IsHTMLElementOfType(startNode.get(), html::TagName::table))
    {
      startOffset = 0;
      startNode = ShareRefPtr(TreeQueries::Next(*startNode));
      if (!startNode)
      {
        return;
      }
    }

    int32 startNodeCaretMaxOffset = CaretMaxOffset(*startNode);
    if (startOffset >= startNodeCaretMaxOffset)
    {
      if (auto *text = DynamicDowncast<dom::Text>(*startNode))
      {
        if (text->Length() > static_cast<size_t>(startNodeCaretMaxOffset))
        {
          DeleteTextFromNode(*text, static_cast<uint32>(startNodeCaretMaxOffset),
                             static_cast<uint32>(text->Length())
                               - static_cast<uint32>(startNodeCaretMaxOffset));
        }
      }
    }

    if (startOffset >= LastOffsetForEditing(*startNode))
    {
      startNode = ShareRefPtr(TreeQueries::NextSkippingChildren(*startNode));
      startOffset = 0;
    }

    // Done adjusting the start.  See if we're all done.
    if (!startNode)
    {
      return;
    }

    if (startNode == _downstreamEnd.DeprecatedNode())
    {
      if (_downstreamEnd.DeprecatedEditingOffset() - startOffset > 0)
      {
        if (auto *text = DynamicDowncast<dom::Text>(*startNode))
        {
          // in a text node that needs to be trimmed
          DeleteTextFromNode(*text, static_cast<uint32>(startOffset),
                             static_cast<uint32>(_downstreamEnd.DeprecatedEditingOffset() - startOffset));
        }
        else
        {
          RemoveChildrenInRange(*startNode, static_cast<uint32>(startOffset),
                                static_cast<uint32>(_downstreamEnd.DeprecatedEditingOffset()));
          _endingPosition = _upstreamStart;
        }
      }

      // The selection to delete is all in one node.
      if (!startNode->Renderer() || (!startOffset && _downstreamEnd.AtLastEditingPositionForNode()))
      {
        RemoveNode(*startNode);
      }
    }
    else
    {
      bool startNodeWasDescendantOfEndNode =
        _upstreamStart.DeprecatedNode()->IsDescendantOf(_downstreamEnd.DeprecatedNode());
      // The selection to delete spans more than one node.
      RefPtr<dom::Node> node = startNode;

      if (startOffset > 0)
      {
        if (auto *text = DynamicDowncast<dom::Text>(*startNode))
        {
          // in a text node that needs to be trimmed
          DeleteTextFromNode(*text, static_cast<uint32>(startOffset),
                             static_cast<uint32>(text->Length()) - static_cast<uint32>(startOffset));
          node = ShareRefPtr(TreeQueries::Next(*startNode));
        }
        else
        {
          auto *child = startNode->FirstChild();
          for (int32 i = 0; child && i < startOffset; ++i)
          {
            child = child->NextSibling();
          }
          node = ShareRefPtr(child);
        }
      }
      else if (startNode == _upstreamEnd.DeprecatedNode())
      {
        if (auto *text = DynamicDowncast<dom::Text>(*startNode))
        {
          DeleteTextFromNode(*text, 0, static_cast<uint32>(_upstreamEnd.DeprecatedEditingOffset()));
        }
      }

      // handle deleting all nodes that are completely selected
      while (node && node != _downstreamEnd.DeprecatedNode() && !_downstreamEnd.IsNull()
             && !_downstreamEnd.IsOrphan())
      {
        if (FirstPositionInOrBeforeNode(node.get()) >= _downstreamEnd)
        {
          // NodeTraversal::nextSkippingChildren just blew past the end position, so stop deleting
          node = nullptr;
          continue;
        }

        if (!_downstreamEnd.DeprecatedNode()->IsDescendantOf(*node))
        {
          auto *parentNode = node->ParentNode();
          if (!parentNode || CanHaveChildrenForEditing(*parentNode))
          {
            RefPtr<dom::Node> nextNode = ShareRefPtr(TreeQueries::NextSkippingChildren(*node));
            // if we just removed a node from the end container, update end position so the
            // check above will work
            UpdatePositionForNodeRemoval(_downstreamEnd, *node);
            RemoveNode(*node);
            node = nextNode;
            continue;
          }
        }

        auto *lastDescendant = TreeQueries::DeepLastChild(*node);
        if (_downstreamEnd.DeprecatedNode() == lastDescendant
            && _downstreamEnd.DeprecatedEditingOffset() >= CaretMaxOffset(*lastDescendant))
        {
          RemoveNode(*node);
          node = nullptr;
        }
        else
        {
          node = ShareRefPtr(TreeQueries::Next(*node));
        }
      }

      if (!_downstreamEnd.IsNull() && !_downstreamEnd.IsOrphan()
          && startNode != _downstreamEnd.DeprecatedNode()
          && !_upstreamStart.DeprecatedNode()->IsDescendantOf(_downstreamEnd.DeprecatedNode())
          && _downstreamEnd.DeprecatedEditingOffset() >= CaretMinOffset(*_downstreamEnd.DeprecatedNode()))
      {
        if (_downstreamEnd.AtLastEditingPositionForNode()
            && !CanHaveChildrenForEditing(*_downstreamEnd.DeprecatedNode()))
        {
          // The node itself is fully selected, not just its contents.  Delete it.
          RemoveNode(*_downstreamEnd.DeprecatedNode());
        }
        else
        {
          if (auto *text = DynamicDowncast<dom::Text>(*_downstreamEnd.DeprecatedNode()))
          {
            // in a text node that needs to be trimmed
            if (_downstreamEnd.DeprecatedEditingOffset() > 0)
            {
              DeleteTextFromNode(*text, 0, static_cast<uint32>(_downstreamEnd.DeprecatedEditingOffset()));
            }
            // Remove children of m_downstreamEnd.deprecatedNode() that come after m_upstreamStart.
            // Don't try to remove children if m_upstreamStart was inside m_downstreamEnd.deprecatedNode()
            // and m_upstreamStart has been removed from the document, because then we don't
            // know how many children to remove.
            // FIXME: Make m_upstreamStart a position we update as we remove content, then we can
            // always know which children to remove.
          }
          else if (!(startNodeWasDescendantOfEndNode && !_upstreamStart.AnchorNode()->IsConnected()))
          {
            uint32 offset = 0;
            if (_upstreamStart.DeprecatedNode()->IsDescendantOf(_downstreamEnd.DeprecatedNode()))
            {
              auto *n = _upstreamStart.DeprecatedNode();
              while (n && n->ParentNode() != _downstreamEnd.DeprecatedNode())
              {
                n = n->ParentNode();
              }
              if (n)
              {
                offset = n->ComputeNodeIndex() + 1;
              }
            }
            RefPtr<dom::Node> downstreamEndNode = ShareRefPtr(_downstreamEnd.DeprecatedNode());
            RemoveChildrenInRange(*downstreamEndNode, offset,
                                  static_cast<uint32>(_downstreamEnd.DeprecatedEditingOffset()));
            _downstreamEnd = dom::MakeDeprecatedLegacyPosition(RefPtr<dom::Node>(downstreamEndNode), offset);
          }
        }
      }
    }
  }

  void DeleteSelectionCommand::FixupWhitespace() noexcept
  {
    (void)GetDocument().UpdateLayoutIgnorePendingStylesheets();

    // The no-break space is longer than the space it replaces, so offsets after it in the same text node
    // move.
    auto replaceWithNonBreakingSpace = [&](dom::Text &textNode, uint32 offset)
    {
      ReplaceTextInNodePreservingMarkers(textNode, offset, 1, NonBreakingSpaceString());
      const auto delta = static_cast<int32>(NonBreakingSpaceString().size()) - 1;
      for (auto *position : {&_endingPosition, &_trailingWhitespace})
      {
        if (position->IsNotNull() && position->DeprecatedNode() == &textNode
            && position->DeprecatedEditingOffset() > static_cast<int32>(offset))
        {
          *position = dom::MakeDeprecatedLegacyPosition(
            ShareRefPtr<dom::Node>(&textNode),
            static_cast<uint32>(position->DeprecatedEditingOffset() + delta));
        }
      }
    };

    // FIXME: isRenderedCharacter should be removed, and we should use VisiblePosition::characterAfter and
    // VisiblePosition::characterBefore
    if (_leadingWhitespace.IsNotNull() && !_leadingWhitespace.IsRenderedCharacter())
    {
      if (auto *textNode = DynamicDowncast<dom::Text>(*_leadingWhitespace.DeprecatedNode()))
      {
        krys_debug_assert(!textNode->Renderer() || textNode->Renderer()->Style().CollapseWhiteSpace());
        replaceWithNonBreakingSpace(*textNode,
                                    static_cast<uint32>(_leadingWhitespace.DeprecatedEditingOffset()));
      }
    }
    if (_trailingWhitespace.IsNotNull() && !_trailingWhitespace.IsRenderedCharacter())
    {
      if (auto *textNode = DynamicDowncast<dom::Text>(*_trailingWhitespace.DeprecatedNode()))
      {
        krys_debug_assert(!textNode->Renderer() || textNode->Renderer()->Style().CollapseWhiteSpace());
        replaceWithNonBreakingSpace(*textNode,
                                    static_cast<uint32>(_trailingWhitespace.DeprecatedEditingOffset()));
      }
    }
  }

  // If a selection starts in one block and ends in another, we have to merge to bring content before the
  // start together with content after the end.
  // FIXME: Asking the editor client whether to move the range after the delete (shouldMoveRangeAfterDelete),
  // with the editing delegates.
  void DeleteSelectionCommand::MergeParagraphs() noexcept
  {
    if (!_mergeBlocksAfterDelete)
    {
      if (_pruneStartBlockIfNecessary)
      {
        // We aren't going to merge into the start block, so remove it if it's empty.
        Prune(_startBlock.get());
        // Removing the start block during a deletion is usually an indication that we need
        // a placeholder, but not in this case.
        _needPlaceholder = false;
      }
      return;
    }

    // It shouldn't have been asked to both try and merge content into the start block and prune it.
    krys_debug_assert(!_pruneStartBlockIfNecessary);

    // FIXME: Deletion should adjust selection endpoints as it removes nodes so that we never get into this
    // state (4099839).
    if (_downstreamEnd.IsNull() || _upstreamStart.IsNull() || _downstreamEnd.IsOrphan()
        || _upstreamStart.IsOrphan())
    {
      return;
    }

    if (_upstreamStart >= _downstreamEnd)
    {
      return;
    }

    VisiblePosition startOfParagraphToMove(_downstreamEnd);
    VisiblePosition mergeDestination(_upstreamStart);

    // m_downstreamEnd's block has been emptied out by deletion.  There is no content inside of it to
    // move, so just remove it.
    RefPtr<dom::Element> endBlock = EnclosingBlock(ShareRefPtr(_downstreamEnd.DeprecatedNode()));
    if (!endBlock)
    {
      return;
    }

    if (!endBlock->Contains(startOfParagraphToMove.DeepEquivalent().DeprecatedNode())
        || !startOfParagraphToMove.DeepEquivalent().DeprecatedNode())
    {
      RemoveNode(*endBlock);
      return;
    }

    // We need to merge into m_upstreamStart's block, but it's been emptied out and collapsed by deletion.
    if (!mergeDestination.DeepEquivalent().DeprecatedNode()
        || !mergeDestination.DeepEquivalent().DeprecatedNode()->IsDescendantOf(
          EnclosingBlock(ShareRefPtr(_upstreamStart.GetContainerNode())).get())
        || _startsAtEmptyLine)
    {
      InsertNodeAt(*CreateHTMLElement(GetDocument(), u8"br"), _upstreamStart);
      mergeDestination = VisiblePosition(_upstreamStart);
    }

    if (mergeDestination == startOfParagraphToMove)
    {
      return;
    }

    VisiblePosition currentStartOfParagraphToMove =
      StartOfParagraph(startOfParagraphToMove, EditingBoundaryCrossingRule::CanSkipOverEditingBoundary);
    VisiblePosition endOfParagraphToMove =
      EndOfParagraph(startOfParagraphToMove, EditingBoundaryCrossingRule::CanSkipOverEditingBoundary);

    if (mergeDestination == endOfParagraphToMove)
    {
      return;
    }

    // The rule for merging into an empty block is: only do so if its farther to the right.
    // FIXME: Consider RTL.
    if (!_startsAtEmptyLine && IsStartOfParagraph(mergeDestination)
        && startOfParagraphToMove.AbsoluteCaretBounds().X() > mergeDestination.AbsoluteCaretBounds().X())
    {
      if (IsHTMLElementOfType(mergeDestination.DeepEquivalent().Downstream().DeprecatedNode(),
                              html::TagName::br))
      {
        RefPtr<dom::Node> nodeToRemove =
          ShareRefPtr(mergeDestination.DeepEquivalent().Downstream().DeprecatedNode());
        RemoveNodeAndPruneAncestors(*nodeToRemove);
        _endingPosition = startOfParagraphToMove.DeepEquivalent();
        return;
      }
    }

    // Block images, tables and horizontal rules cannot be made inline with content at mergeDestination.  If
    // there is any (!isStartOfParagraph(mergeDestination)), don't merge, just move the caret to just before
    // the selection we deleted. See https://bugs.webkit.org/show_bug.cgi?id=25439
    if (IsRenderedAsNonInlineTableImageOrHR(startOfParagraphToMove.DeepEquivalent().DeprecatedNode())
        && !IsStartOfParagraph(mergeDestination))
    {
      _endingPosition = _upstreamStart;
      return;
    }

    auto range = MakeSimpleRange(VisiblePositionRange {startOfParagraphToMove, endOfParagraphToMove});
    if (!range)
    {
      return;
    }

    // moveParagraphs will insert placeholders if it removes blocks that would require their use, don't let
    // block removals that it does cause the insertion of *another* placeholder.
    bool needPlaceholder = _needPlaceholder;
    bool paragraphToMergeIsEmpty = (currentStartOfParagraphToMove == endOfParagraphToMove);
    MoveParagraph(currentStartOfParagraphToMove, endOfParagraphToMove, mergeDestination, false,
                  !paragraphToMergeIsEmpty);
    _needPlaceholder = needPlaceholder;
    // The endingPosition was likely clobbered by the move, so recompute it (moveParagraph selects the moved
    // paragraph).

    // FIXME (Bug 211793): endingSelection() becomes disconnected in moveParagraph
    if (auto *anchorNode = EndingSelection().Start().AnchorNode(); anchorNode && anchorNode->IsConnected())
    {
      _endingPosition = EndingSelection().Start();
    }
  }

  void DeleteSelectionCommand::RemovePreviouslySelectedEmptyTableRows() noexcept
  {
    // DeleteSelectionCommand::removeNode does not remove rows but only empties them in preparation for this
    // function. Instead, DeleteSelectionCommand::removeNodeUpdatingStates is used below, which calls a raw
    // CompositeEditCommand::removeNode and adjusts selection.
    if (_endTableRow && _endTableRow->IsConnected() && _endTableRow != _startTableRow)
    {
      RefPtr<dom::Node> row = ShareRefPtr(_endTableRow->PreviousSibling());
      while (row && row != _startTableRow)
      {
        RefPtr<dom::Node> previousRow = ShareRefPtr(row->PreviousSibling());
        if (IsTableRowEmpty(*row))
        {
          RemoveNodeUpdatingStates(*row, DoNotAssumeContentIsAlwaysEditable);
        }
        row = krys::move(previousRow);
      }
    }

    // Remove empty rows after the start row.
    if (_startTableRow && _startTableRow->IsConnected() && _startTableRow != _endTableRow)
    {
      RefPtr<dom::Node> row = ShareRefPtr(_startTableRow->NextSibling());
      while (row && row != _endTableRow)
      {
        RefPtr<dom::Node> nextRow = ShareRefPtr(row->NextSibling());
        if (IsTableRowEmpty(*row))
        {
          RemoveNodeUpdatingStates(*row, DoNotAssumeContentIsAlwaysEditable);
        }
        row = krys::move(nextRow);
      }
    }

    RefPtr<dom::Node> endTableRow = _startTableRow;
    if (endTableRow && endTableRow->IsConnected() && endTableRow != _startTableRow)
    {
      if (IsTableRowEmpty(*endTableRow))
      {
        // Don't remove m_endTableRow if it's where we're putting the ending selection.
        if (!_endingPosition.DeprecatedNode()->IsDescendantOf(*endTableRow))
        {
          // FIXME: We probably shouldn't remove m_endTableRow unless it's fully selected, even if it is
          // empty. We'll need to start adjusting the selection endpoints during deletion to know whether or
          // not m_endTableRow was fully selected here.
          RemoveNodeUpdatingStates(*endTableRow, DoNotAssumeContentIsAlwaysEditable);
        }
      }
    }
  }

  void DeleteSelectionCommand::CalculateTypingStyleAfterDelete() noexcept
  {
    if (!_typingStyle)
    {
      return;
    }

    // Compute the difference between the style before the delete and the style now
    // after the delete has been done. Set this style on the frame, so other editing
    // commands being composed with this one will work, and also cache it on the command,
    // so the Frame::appliedEditing can set it after the whole composite command
    // has completed.

    // If we deleted into a blockquote, but are now no longer in a blockquote, use the alternate typing style
    if (_deleteIntoBlockquoteStyle
        && !EnclosingNodeOfType(_endingPosition, IsMailBlockquote,
                                EditingBoundaryCrossingRule::CanCrossEditingBoundary))
    {
      _typingStyle = _deleteIntoBlockquoteStyle;
    }
    _deleteIntoBlockquoteStyle = nullptr;

    _typingStyle->PrepareToApplyAt(_endingPosition);
    if (_typingStyle->IsEmpty())
    {
      _typingStyle = nullptr;
    }
    // This is where we've deleted all traces of a style but not a whole paragraph (that's handled above).
    // In this case if we start typing, the new characters should have the same style as the just deleted
    // ones, but, if we change the selection, come back and start typing that style should be lost.  Also see
    // preserveTypingStyle() below.
    GetDocument().Selection().SetTypingStyle(RefPtr<EditingStyle>(_typingStyle));
  }

  void DeleteSelectionCommand::ClearTransientState() noexcept
  {
    _selectionToDelete = VisibleSelection();
    _upstreamStart.Clear();
    _downstreamStart.Clear();
    _upstreamEnd.Clear();
    _downstreamEnd.Clear();
    _endingPosition.Clear();
    _leadingWhitespace.Clear();
    _trailingWhitespace.Clear();
  }

  // This method removes div elements with no attributes that have only one child or no children at all.
  void DeleteSelectionCommand::RemoveRedundantBlocks() noexcept
  {
    RefPtr<dom::Node> node = ShareRefPtr(_endingPosition.GetContainerNode());
    if (!node)
    {
      return;
    }
    RefPtr<dom::Element> rootNode = ShareRefPtr(node->RootEditableElement());

    while (node && node != rootNode)
    {
      if (IsRemovableBlock(node.get()))
      {
        if (node == _endingPosition.AnchorNode())
        {
          UpdatePositionForNodeRemovalPreservingChildren(_endingPosition, *node);
        }

        CompositeEditCommand::RemoveNodePreservingChildren(*node);
        node = ShareRefPtr(_endingPosition.AnchorNode());
      }
      else
      {
        node = ShareRefPtr<dom::Node>(node->ParentNode());
      }
    }
  }

  // FIXME: Not rebalancing whitespace in secure fields (-webkit-text-security), which isn't supported.
  void DeleteSelectionCommand::DoApply() noexcept
  {
    // If selection has not been set to a custom selection when the command was created,
    // use the current ending selection.
    if (!_hasSelectionToDelete)
    {
      _selectionToDelete = EndingSelection();
    }

    if (!_selectionToDelete.IsNonOrphanedRange() || !_selectionToDelete.IsContentEditable())
    {
      return;
    }

    // If the deletion is occurring in a text field, and we're not deleting to replace the selection, then let
    // the frame call across the bridge to notify the form delegate.
    if (!_replace)
    {
      if (RefPtr textControl = ShareRefPtr(html::EnclosingTextFormControl(_selectionToDelete.Start()));
          textControl && textControl->Focused())
      {
        GetDocument().GetEditor().TextWillBeDeletedInTextField(*textControl);
      }
    }

    // save this to later make the selection with
    auto affinity = _selectionToDelete.GetAffinity();

    _needPlaceholder = IsStartOfParagraph(_selectionToDelete.VisibleStart(),
                                          EditingBoundaryCrossingRule::CanCrossEditingBoundary)
                       && IsEndOfParagraph(_selectionToDelete.VisibleEnd(),
                                           EditingBoundaryCrossingRule::CanCrossEditingBoundary)
                       && !LineBreakExistsAtVisiblePosition(_selectionToDelete.VisibleEnd());
    if (_needPlaceholder)
    {
      // Don't need a placeholder when deleting a selection that starts just before a table
      // and ends inside it (we do need placeholders to hold open empty cells, but that's
      // handled elsewhere).
      if (auto table = IsLastPositionBeforeTable(_selectionToDelete.VisibleStart()))
      {
        if (_selectionToDelete.End().DeprecatedNode()->IsDescendantOf(*table))
        {
          _needPlaceholder = false;
        }
      }
    }

    // set up our state
    if (!InitializePositionData())
    {
      return;
    }

    // Delete any text that may hinder our ability to fixup whitespace after the delete
    DeleteInsignificantTextDownstream(_trailingWhitespace);

    SaveTypingStyleState();

    // deleting just a BR is handled specially, at least because we do not
    // want to replace it with a placeholder BR!
    if (HandleSpecialCaseBRDelete())
    {
      CalculateTypingStyleAfterDelete();
      SetEndingSelection(VisibleSelection(_endingPosition, affinity, EndingSelection().GetDirectionality()));
      ClearTransientState();
      RebalanceWhitespace();
      return;
    }

    HandleGeneralDelete();

    FixupWhitespace();

    MergeParagraphs();

    RemovePreviouslySelectedEmptyTableRows();

    if (_needPlaceholder)
    {
      if (_sanitizeMarkup)
      {
        RemoveRedundantBlocks();
      }

      // FIXME (Bug 212723): m_endingPosition becomes disconnected in moveParagraph()
      // because it is ancestor of the deleted element and gets pruned in removeNodeAndPruneAncestors().
      // Either removeNodeAndPruneAncestors() should not remove ending position or we should find
      // a different ending position.
      if (!_endingPosition.GetContainerNode() || !_endingPosition.GetContainerNode()->IsConnected())
      {
        return;
      }
      InsertNodeAt(*CreateHTMLElement(GetDocument(), u8"br"), _endingPosition);
    }

    RebalanceWhitespaceAt(_endingPosition);

    CalculateTypingStyleAfterDelete();

    SetEndingSelection(
      VisibleSelection(VisiblePosition(_endingPosition, affinity), EndingSelection().GetDirectionality()));
    ClearTransientState();
  }

  // Normally deletion doesn't preserve the typing style that was present before it.  For example,
  // type a character, Bold, then delete the character and start typing.  The Bold typing style shouldn't
  // stick around.  Deletion should preserve a typing style that *it* sets, however.
  bool DeleteSelectionCommand::PreservesTypingStyle() const noexcept
  {
    return static_cast<bool>(_typingStyle);
  }
}

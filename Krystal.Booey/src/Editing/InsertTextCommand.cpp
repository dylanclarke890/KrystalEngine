#include "Krystal.Booey/Editing/InsertTextCommand.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/EditingStyle.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include <algorithm>

namespace krys::boo
{
  InsertTextCommand::InsertTextCommand(dom::Document &document, utf8_stringview text,
                                       AllowPasswordEcho allowPasswordEcho, bool selectInsertedText,
                                       RebalanceType rebalanceType, EditAction editingAction) noexcept
      : CompositeEditCommand(document, editingAction), _text(text), _allowPasswordEcho(allowPasswordEcho),
        _selectInsertedText(selectInsertedText), _rebalanceType(rebalanceType)
  {
  }

  InsertTextCommand::InsertTextCommand(dom::Document &document, utf8_stringview text,
                                       Ref<TextInsertionMarkerSupplier> &&markerSupplier,
                                       EditAction editingAction) noexcept
      : CompositeEditCommand(document, editingAction), _text(text), _selectInsertedText(false),
        _rebalanceType(RebalanceLeadingAndTrailingWhitespaces), _markerSupplier(krys::move(markerSupplier))
  {
  }

  Ref<InsertTextCommand> InsertTextCommand::Create(dom::Document &document, utf8_stringview text,
                                                   AllowPasswordEcho allowPasswordEcho,
                                                   bool selectInsertedText, RebalanceType rebalanceType,
                                                   EditAction editingAction) noexcept
  {
    return CreateRef<InsertTextCommand>(document, text, allowPasswordEcho, selectInsertedText, rebalanceType,
                                        editingAction);
  }

  Ref<InsertTextCommand>
    InsertTextCommand::CreateWithMarkerSupplier(dom::Document &document, utf8_stringview text,
                                                Ref<TextInsertionMarkerSupplier> &&markerSupplier,
                                                EditAction editingAction) noexcept
  {
    return CreateRef<InsertTextCommand>(document, text, krys::move(markerSupplier), editingAction);
  }

  // FIXME: An editing text node (createEditingTextNode, which echoes passwords), with password fields.
  dom::Position InsertTextCommand::PositionInsideTextNode(const dom::Position &position) noexcept
  {
    dom::Position pos = position;
    if (ParentTabSpanNode(pos.AnchorNode()))
    {
      auto textNode = GetDocument().CreateTextNode(utf8_string());
      InsertNodeAtTabSpanPosition(*textNode, pos);
      return dom::FirstPositionInNode(textNode.get());
    }

    // Prepare for text input by looking at the specified position.
    // It may be necessary to insert a text node to receive characters.
    if (!Is<dom::Text>(*pos.GetContainerNode()))
    {
      auto textNode = GetDocument().CreateTextNode(utf8_string());
      InsertNodeAt(*textNode, pos);
      return dom::FirstPositionInNode(textNode.get());
    }

    return pos;
  }

  void InsertTextCommand::SetEndingSelectionWithoutValidation(const dom::Position &startPosition,
                                                              const dom::Position &endPosition) noexcept
  {
    // We could have inserted a part of composed character sequence,
    // so we are basically treating ending selection as a range to avoid validation.
    // <http://bugs.webkit.org/show_bug.cgi?id=15781>
    VisibleSelection forcedEndingSelection;
    forcedEndingSelection.SetWithoutValidation(startPosition, endPosition);
    forcedEndingSelection.SetDirectionality(EndingSelection().GetDirectionality());
    SetEndingSelection(forcedEndingSelection);
  }

  // This avoids the expense of a full fledged delete operation, and avoids a layout that typically results
  // from text removal.
  bool InsertTextCommand::PerformTrivialReplace(utf8_stringview text, bool selectInsertedText) noexcept
  {
    if (!EndingSelection().IsRange())
    {
      return false;
    }

    if (std::ranges::any_of(text, [](char8_t c) { return c == u8'\t' || c == u8' ' || c == u8'\n'; }))
    {
      return false;
    }

    dom::Position start = EndingSelection().Start();
    dom::Position endPosition = ReplaceSelectedTextInNode(text);
    if (endPosition.IsNull())
    {
      return false;
    }

    SetEndingSelectionWithoutValidation(start, endPosition);
    if (!selectInsertedText)
    {
      SetEndingSelection(
        VisibleSelection(EndingSelection().VisibleEnd(), EndingSelection().GetDirectionality()));
    }

    return true;
  }

  bool InsertTextCommand::PerformOverwrite(utf8_stringview text, bool selectInsertedText) noexcept
  {
    dom::Position start = EndingSelection().Start();
    RefPtr<dom::Text> textNode = ShareRefPtr(start.ContainerText());
    if (!textNode)
    {
      return false;
    }

    auto count = static_cast<uint32>(
      std::min(text.size(), textNode->Length() - static_cast<size_t>(start.OffsetInContainerNode())));
    if (!count)
    {
      return false;
    }

    ReplaceTextInNode(*textNode, static_cast<uint32>(start.OffsetInContainerNode()), count, text);

    dom::Position endPosition =
      dom::Position(RefPtr<dom::Text>(textNode),
                    static_cast<uint32>(start.OffsetInContainerNode()) + static_cast<uint32>(text.size()));
    SetEndingSelectionWithoutValidation(start, endPosition);
    if (!selectInsertedText)
    {
      SetEndingSelection(
        VisibleSelection(EndingSelection().VisibleEnd(), EndingSelection().GetDirectionality()));
    }

    return true;
  }

  // FIXME: Writing suggestions (isInsertingTextForWritingSuggestion), with writing suggestions.
  void InsertTextCommand::DoApply() noexcept
  {
    krys_debug_assert(_text.find(u8'\n') == utf8_string::npos);

    if (EndingSelection().IsNoneOrOrphaned())
    {
      return;
    }

    // Delete the current selection.
    // FIXME: This delete operation blows away the typing style.
    if (EndingSelection().IsRange())
    {
      if (PerformTrivialReplace(_text, _selectInsertedText))
      {
        return;
      }
      DeleteSelection(false, true, true, false, false);
      // deleteSelection eventually makes a new endingSelection out of a Position. If that Position doesn't
      // have a renderer (e.g. it is on a <frameset> in the DOM), the VisibleSelection cannot be canonicalized
      // to anything other than NoSelection. The rest of this function requires a real endingSelection, so
      // bail out.
      if (EndingSelection().IsNoneOrOrphaned())
      {
        return;
      }
    }
    else if (GetDocument().GetEditor().IsOverwriteModeEnabled())
    {
      if (PerformOverwrite(_text, _selectInsertedText))
      {
        return;
      }
    }

    dom::Position startPosition(EndingSelection().Start());

    dom::Position placeholder;
    // We want to remove preserved newlines and brs that will collapse (and thus become unnecessary) when
    // content is inserted just before them.
    // FIXME: We shouldn't really have to do this, but removing placeholders is a workaround for 9661.
    // If the caret is just before a placeholder, downstream will normalize the caret to it.
    dom::Position downstream(startPosition.Downstream());
    if (LineBreakExistsAtPosition(downstream))
    {
      // FIXME: This doesn't handle placeholders at the end of anonymous blocks.
      VisiblePosition caret(startPosition);
      if (IsEndOfBlock(caret) && IsStartOfParagraph(caret))
      {
        placeholder = downstream;
      }
      // Don't remove the placeholder yet, otherwise the block we're inserting into would collapse before
      // we get a chance to insert into it.  We check for a placeholder now, though, because doing so requires
      // the creation of a VisiblePosition, and if we did that post-insertion it would force a layout.
    }

    // Insert the character at the leftmost candidate.
    startPosition = startPosition.Upstream();

    // It is possible for the node that contains startPosition to contain only unrendered whitespace,
    // and so deleteInsignificantText could remove it.  Save the position before the node in case that
    // happens.
    dom::Position positionBeforeStartNode(dom::PositionInParentBeforeNode(startPosition.GetContainerNode()));

    DeleteInsignificantText(startPosition, startPosition.Downstream());

    if (!startPosition.AnchorNode()->IsConnected())
    {
      startPosition = positionBeforeStartNode;
    }
    if (!startPosition.IsCandidate())
    {
      startPosition = startPosition.Downstream();
    }

    startPosition = PositionAvoidingSpecialElementBoundary(startPosition);
    if (EndingSelection().IsNoneOrOrphaned())
    {
      return;
    }

    dom::Position endPosition;

    if (_text == u8"\t")
    {
      endPosition = InsertTab(startPosition);
      startPosition = endPosition.Previous();
      if (placeholder.IsNotNull())
      {
        RemovePlaceholderAt(placeholder);
      }
    }
    else
    {
      // Make sure the document is set up to receive _text
      startPosition = PositionInsideTextNode(startPosition);
      krys_debug_assert(startPosition.GetAnchorType() == dom::Position::AnchorType::PositionIsOffsetInAnchor);
      krys_debug_assert(startPosition.GetContainerNode());
      krys_debug_assert(Is<dom::Text>(*startPosition.GetContainerNode()));
      if (placeholder.IsNotNull())
      {
        RemovePlaceholderAt(placeholder);
      }
      RefPtr<dom::Text> textNode = ShareRefPtr(startPosition.ContainerText());
      const auto offset = static_cast<uint32>(startPosition.OffsetInContainerNode());

      InsertTextIntoNode(*textNode, offset, _text, _allowPasswordEcho);
      endPosition = dom::Position(RefPtr<dom::Text>(textNode), offset + static_cast<uint32>(_text.size()));
      if (_markerSupplier)
      {
        _markerSupplier->AddMarkersToTextNode(*textNode, offset, _text);
      }

      // A rebalanced run can change length (a no-break space is two bytes), so the positions move across it.
      auto adjustPositions = [&](const Maybe<WhitespaceRebalance> &rebalance)
      {
        if (rebalance)
        {
          startPosition = rebalance->Adjust(startPosition);
          endPosition = rebalance->Adjust(endPosition);
        }
      };
      if (_rebalanceType == RebalanceLeadingAndTrailingWhitespaces)
      {
        // The insertion may require adjusting adjacent whitespace, if it is present.
        adjustPositions(RebalanceWhitespaceAt(endPosition));
        // Rebalancing on both sides isn't necessary if we've inserted only spaces.
        if (!ShouldRebalanceLeadingWhitespaceFor(_text))
        {
          adjustPositions(RebalanceWhitespaceAt(startPosition));
        }
      }
      else
      {
        krys_debug_assert(_rebalanceType == RebalanceAllWhitespaces);
        krys_debug_assert(TextNodeForRebalance(startPosition) == TextNodeForRebalance(endPosition));
        if (auto textForRebalance = TextNodeForRebalance(startPosition))
        {
          krys_debug_assert(textForRebalance == textNode);
          adjustPositions(RebalanceWhitespaceOnTextSubstring(*textNode, startPosition.OffsetInContainerNode(),
                                                             endPosition.OffsetInContainerNode()));
        }
      }
    }

    SetEndingSelectionWithoutValidation(startPosition, endPosition);

    RefPtr<EditingStyle> typingStyle = ShareRefPtr(GetDocument().Selection().TypingStyle());

    if (typingStyle)
    {
      typingStyle->PrepareToApplyAt(endPosition, EditingStyle::ShouldPreserveWritingDirection::Yes);
      if (!typingStyle->IsEmpty())
      {
        ApplyStyle(typingStyle.get());
      }
    }

    if (!_selectInsertedText)
    {
      SetEndingSelection(VisibleSelection(EndingSelection().End(), EndingSelection().GetAffinity(),
                                          EndingSelection().GetDirectionality()));
    }
  }

  dom::Position InsertTextCommand::InsertTab(const dom::Position &position) noexcept
  {
    dom::Position insertPos = VisiblePosition(position).DeepEquivalent();
    if (insertPos.IsNull())
    {
      return position;
    }

    RefPtr<dom::Node> node = ShareRefPtr(insertPos.GetContainerNode());
    uint32 offset = Is<dom::Text>(*node) ? static_cast<uint32>(insertPos.OffsetInContainerNode()) : 0;

    // keep tabs coalesced in tab span
    if (ParentTabSpanNode(node.get()))
    {
      auto &textNode = Downcast<dom::Text>(*node);
      InsertTextIntoNode(textNode, offset, u8"\t");
      return dom::Position(ShareRefPtr(&textNode), offset + 1);
    }

    // create new tab span
    auto spanNode = CreateTabSpanElement(GetDocument());

    // place it
    if (auto *textNode = DynamicDowncast<dom::Text>(*node))
    {
      if (offset >= textNode->Length())
      {
        InsertNodeAfter(*spanNode, *textNode);
      }
      else
      {
        // split node to make room for the span
        // NOTE: splitTextNode uses textNode for the
        // second node in the split, so we need to
        // insert the span before it.
        if (offset > 0)
        {
          SplitTextNode(*textNode, offset);
        }
        (void)InsertNodeBefore(*spanNode, *textNode);
      }
    }
    else
    {
      InsertNodeAt(*spanNode, insertPos);
    }

    // return the position following the new tab
    return dom::LastPositionInNode(spanNode.get());
  }
}

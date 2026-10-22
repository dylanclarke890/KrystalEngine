#include "Krystal.Booey/Editing/TypingCommand.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/StaticRange.hpp"
#include "Krystal.Booey/Editing/BreakBlockquoteCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/InsertLineBreakCommand.hpp"
#include "Krystal.Booey/Editing/InsertParagraphSeparatorCommand.hpp"
#include "Krystal.Booey/Editing/InsertTextCommand.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"

namespace krys::boo
{
  class TypingCommandLineOperation
  {
  private:
    TypingCommand &_typingCommand;
    bool _selectInsertedText;
    utf8_stringview _text;

  public:
    TypingCommandLineOperation(TypingCommand &typingCommand, bool selectInsertedText,
                               utf8_stringview text) noexcept
        : _typingCommand(typingCommand), _selectInsertedText(selectInsertedText), _text(text)
    {
    }

    void operator()(size_t lineOffset, size_t lineLength, bool isLastLine) const noexcept
    {
      if (isLastLine)
      {
        if (!lineOffset || lineLength > 0)
        {
          _typingCommand.InsertTextRunWithoutNewlines(_text.substr(lineOffset, lineLength),
                                                      _selectInsertedText);
        }
      }
      else
      {
        if (lineLength > 0)
        {
          _typingCommand.InsertTextRunWithoutNewlines(_text.substr(lineOffset, lineLength), false);
        }
        _typingCommand.InsertParagraphSeparator();
      }
    }
  };

  namespace
  {
    KRYS_NODISCARD EditAction EditActionForTypingCommand(TypingCommand::Type command,
                                                         TextGranularity granularity,
                                                         TypingCommand::TextCompositionType compositionType,
                                                         bool isAutocompletion) noexcept
    {
      if (compositionType == TypingCommand::TextCompositionType::Pending)
      {
        if (command == TypingCommand::Type::InsertText)
        {
          return EditAction::TypingInsertPendingComposition;
        }
        if (command == TypingCommand::Type::DeleteSelection)
        {
          return EditAction::TypingDeletePendingComposition;
        }
        krys_debug_assert(false);
      }

      if (compositionType == TypingCommand::TextCompositionType::Final)
      {
        if (command == TypingCommand::Type::InsertText)
        {
          return EditAction::TypingInsertFinalComposition;
        }
        if (command == TypingCommand::Type::DeleteSelection)
        {
          return EditAction::TypingDeleteFinalComposition;
        }
        krys_debug_assert(false);
      }

      switch (command)
      {
        case TypingCommand::Type::DeleteSelection: return EditAction::TypingDeleteSelection;
        case TypingCommand::Type::DeleteKey:
        {
          if (granularity == TextGranularity::WordGranularity)
          {
            return EditAction::TypingDeleteWordBackward;
          }
          if (granularity == TextGranularity::LineBoundary)
          {
            return EditAction::TypingDeleteLineBackward;
          }
          return EditAction::TypingDeleteBackward;
        }
        case TypingCommand::Type::ForwardDeleteKey:
          if (granularity == TextGranularity::WordGranularity)
          {
            return EditAction::TypingDeleteWordForward;
          }
          if (granularity == TextGranularity::LineBoundary)
          {
            return EditAction::TypingDeleteLineForward;
          }
          return EditAction::TypingDeleteForward;
        case TypingCommand::Type::InsertText:
          return isAutocompletion ? EditAction::InsertReplacement : EditAction::TypingInsertText;
        case TypingCommand::Type::InsertLineBreak: return EditAction::TypingInsertLineBreak;
        case TypingCommand::Type::InsertParagraphSeparator:
        case TypingCommand::Type::InsertParagraphSeparatorInQuotedContent:
          return EditAction::TypingInsertParagraph;
      }
      return EditAction::Unspecified;
    }

    KRYS_NODISCARD bool EditActionIsDeleteByTyping(EditAction action) noexcept
    {
      switch (action)
      {
        case EditAction::TypingDeleteSelection:
        case EditAction::TypingDeleteBackward:
        case EditAction::TypingDeleteWordBackward:
        case EditAction::TypingDeleteLineBackward:
        case EditAction::TypingDeleteForward:
        case EditAction::TypingDeleteWordForward:
        case EditAction::TypingDeleteLineForward:  return true;
        default:                                   return false;
      }
    }
  }

  TypingCommand::TypingCommand(dom::Document &document, Type commandType, utf8_stringview textToInsert,
                               EnumSet<Option> options, TextGranularity granularity,
                               TextCompositionType compositionType) noexcept
      : TextInsertionBaseCommand(document,
                                 EditActionForTypingCommand(commandType, granularity, compositionType,
                                                            options.Contains(Option::IsAutocompletion))),
        _commandType(commandType), _textToInsert(textToInsert), _currentTextToInsert(textToInsert),
        _selectInsertedText(options.Contains(Option::SelectInsertedText)),
        _smartDelete(options.Contains(Option::SmartDelete)), _granularity(granularity),
        _compositionType(compositionType), _shouldAddToKillRing(options.Contains(Option::AddsToKillRing)),
        _isAutocompletion(options.Contains(Option::IsAutocompletion)),
        _shouldRetainAutocorrectionIndicator(options.Contains(Option::RetainAutocorrectionIndicator)),
        _shouldPreventSpellChecking(options.Contains(Option::PreventSpellChecking))
  {
    _currentTypingEditAction = EditingAction();
    UpdatePreservesTypingStyle(_commandType);
  }

  Ref<TypingCommand> TypingCommand::Create(dom::Document &document, Type command, utf8_stringview text,
                                           EnumSet<Option> options, TextGranularity granularity,
                                           TextCompositionType compositionType) noexcept
  {
    return CreateRef<TypingCommand>(document, command, text, options, granularity, compositionType);
  }

  Ref<TypingCommand> TypingCommand::Create(dom::Document &document, Type command, utf8_stringview text,
                                           EnumSet<Option> options,
                                           TextCompositionType compositionType) noexcept
  {
    return CreateRef<TypingCommand>(document, command, text, options, TextGranularity::CharacterGranularity,
                                    compositionType);
  }

  void TypingCommand::DeleteSelection(dom::Document &document, EnumSet<Option> options,
                                      TextCompositionType compositionType) noexcept
  {
    if (!document.Selection().IsRange())
    {
      return;
    }

    if (auto lastTypingCommand = LastTypingCommandIfStillOpenForTyping(document))
    {
      lastTypingCommand->SetIsAutocompletion(options.Contains(Option::IsAutocompletion));
      lastTypingCommand->SetCompositionType(compositionType);
      lastTypingCommand->SetShouldPreventSpellChecking(options.Contains(Option::PreventSpellChecking));
      lastTypingCommand->DeleteSelection(options.Contains(Option::SmartDelete));
      return;
    }

    Create(document, Type::DeleteSelection, {}, options, compositionType)->Apply();
  }

  void TypingCommand::DeleteKeyPressed(dom::Document &document, EnumSet<Option> options,
                                       TextGranularity granularity) noexcept
  {
    if (granularity == TextGranularity::CharacterGranularity)
    {
      if (auto lastTypingCommand = LastTypingCommandIfStillOpenForTyping(document))
      {
        UpdateSelectionIfDifferentFromCurrentSelection(lastTypingCommand.get(), document);
        lastTypingCommand->SetIsAutocompletion(options.Contains(Option::IsAutocompletion));
        lastTypingCommand->SetCompositionType(TextCompositionType::None);
        lastTypingCommand->SetShouldPreventSpellChecking(options.Contains(Option::PreventSpellChecking));
        lastTypingCommand->DeleteKeyPressed(granularity, options.Contains(Option::AddsToKillRing));
        return;
      }
    }

    Create(document, Type::DeleteKey, {}, options, granularity)->Apply();
  }

  void TypingCommand::ForwardDeleteKeyPressed(dom::Document &document, EnumSet<Option> options,
                                              TextGranularity granularity) noexcept
  {
    // FIXME: Forward delete in TextEdit appears to open and close a new typing command.
    if (granularity == TextGranularity::CharacterGranularity)
    {
      if (auto lastTypingCommand = LastTypingCommandIfStillOpenForTyping(document))
      {
        UpdateSelectionIfDifferentFromCurrentSelection(lastTypingCommand.get(), document);
        lastTypingCommand->SetIsAutocompletion(options.Contains(Option::IsAutocompletion));
        lastTypingCommand->SetCompositionType(TextCompositionType::None);
        lastTypingCommand->SetShouldPreventSpellChecking(options.Contains(Option::PreventSpellChecking));
        lastTypingCommand->ForwardDeleteKeyPressed(granularity, options.Contains(Option::AddsToKillRing));
        return;
      }
    }

    Create(document, Type::ForwardDeleteKey, {}, options, granularity)->Apply();
  }

  void TypingCommand::UpdateSelectionIfDifferentFromCurrentSelection(TypingCommand *typingCommand,
                                                                     dom::Document &document) noexcept
  {
    VisibleSelection currentSelection = document.Selection().Selection();
    if (currentSelection == typingCommand->EndingSelection())
    {
      return;
    }

    typingCommand->SetStartingSelection(currentSelection);
    typingCommand->SetEndingSelection(currentSelection);
  }

  // FIXME: Updating the spelling and grammar markers of the words the typing touches
  // (updateMarkersForWordsAffectedByEditing), with text checking.
  void TypingCommand::InsertText(dom::Document &document, utf8_stringview text, dom::Event *triggeringEvent,
                                 EnumSet<Option> options, TextCompositionType composition) noexcept
  {
    const auto &selection = document.Selection().Selection();
    InsertText(document, text, triggeringEvent, selection, options, composition);
  }

  // FIXME: We shouldn't need to take selectionForInsertion. It should be identical to FrameSelection's
  // current selection.
  void TypingCommand::InsertText(dom::Document &document, utf8_stringview text, dom::Event *triggeringEvent,
                                 const VisibleSelection &selectionForInsertion, EnumSet<Option> options,
                                 TextCompositionType compositionType) noexcept
  {
    VisibleSelection currentSelection = document.Selection().Selection();

    auto newText = DispatchBeforeTextInsertedEvent(text, selectionForInsertion,
                                                   compositionType == TextCompositionType::Pending);

    bool triggeringEventIsUntrusted = triggeringEvent && !triggeringEvent->IsTrusted();

    // Set the starting and ending selection appropriately if we are using a selection
    // that is different from the current selection.  In the future, we should change EditCommand
    // to deal with custom selections in a general way that can be used by all of the commands.
    if (auto lastTypingCommand = LastTypingCommandIfStillOpenForTyping(document))
    {
      if (lastTypingCommand->EndingSelection() != selectionForInsertion)
      {
        lastTypingCommand->SetStartingSelection(selectionForInsertion);
        lastTypingCommand->SetEndingSelection(selectionForInsertion);
      }

      lastTypingCommand->SetIsAutocompletion(options.Contains(Option::IsAutocompletion));
      lastTypingCommand->SetCompositionType(compositionType);
      lastTypingCommand->SetShouldRetainAutocorrectionIndicator(
        options.Contains(Option::RetainAutocorrectionIndicator));
      lastTypingCommand->SetShouldPreventSpellChecking(options.Contains(Option::PreventSpellChecking));
      lastTypingCommand->SetTriggeringEventIsUntrusted(triggeringEventIsUntrusted);
      lastTypingCommand->InsertText(newText, options.Contains(Option::SelectInsertedText));
      return;
    }

    RefPtr<LocalFrame> frame = ShareRefPtr(document.Frame());
    auto command = Create(document, Type::InsertText, newText, options, compositionType);
    command->SetTriggeringEventIsUntrusted(triggeringEventIsUntrusted);
    ApplyTextInsertionCommand(frame.get(), *command, selectionForInsertion, currentSelection);
  }

  void TypingCommand::InsertLineBreak(dom::Document &document, EnumSet<Option> options) noexcept
  {
    if (auto lastTypingCommand = LastTypingCommandIfStillOpenForTyping(document))
    {
      lastTypingCommand->SetIsAutocompletion(options.Contains(Option::IsAutocompletion));
      lastTypingCommand->SetCompositionType(TextCompositionType::None);
      lastTypingCommand->SetShouldRetainAutocorrectionIndicator(
        options.Contains(Option::RetainAutocorrectionIndicator));
      lastTypingCommand->InsertLineBreak();
      return;
    }

    Create(document, Type::InsertLineBreak, {}, options)->Apply();
  }

  void TypingCommand::InsertParagraphSeparatorInQuotedContent(dom::Document &document) noexcept
  {
    if (auto lastTypingCommand = LastTypingCommandIfStillOpenForTyping(document))
    {
      lastTypingCommand->SetIsAutocompletion(false);
      lastTypingCommand->SetCompositionType(TextCompositionType::None);
      lastTypingCommand->InsertParagraphSeparatorInQuotedContent();
      return;
    }

    Create(document, Type::InsertParagraphSeparatorInQuotedContent)->Apply();
  }

  void TypingCommand::InsertParagraphSeparator(dom::Document &document, EnumSet<Option> options) noexcept
  {
    if (auto lastTypingCommand = LastTypingCommandIfStillOpenForTyping(document))
    {
      lastTypingCommand->SetIsAutocompletion(options.Contains(Option::IsAutocompletion));
      lastTypingCommand->SetCompositionType(TextCompositionType::None);
      lastTypingCommand->SetShouldRetainAutocorrectionIndicator(
        options.Contains(Option::RetainAutocorrectionIndicator));
      lastTypingCommand->InsertParagraphSeparator();
      return;
    }

    Create(document, Type::InsertParagraphSeparator, {}, options)->Apply();
  }

  RefPtr<TypingCommand> TypingCommand::LastTypingCommandIfStillOpenForTyping(dom::Document &document) noexcept
  {
    auto *lastEditCommand = DynamicDowncast<TypingCommand>(document.GetEditor().LastEditCommand());
    if (!lastEditCommand || !lastEditCommand->IsOpenForMoreTyping())
    {
      return nullptr;
    }
    return ShareRefPtr(lastEditCommand);
  }

  bool TypingCommand::ShouldDeferWillApplyCommandUntilAddingTypingCommand() const noexcept
  {
    return !_isHandlingInitialTypingCommand || EditActionIsDeleteByTyping(EditingAction());
  }

  void TypingCommand::CloseTyping(dom::Document &document) noexcept
  {
    if (auto lastTypingCommand = LastTypingCommandIfStillOpenForTyping(document))
    {
      lastTypingCommand->CloseTyping();
    }
  }

  bool TypingCommand::WillApplyCommand() noexcept
  {
    if (ShouldDeferWillApplyCommandUntilAddingTypingCommand())
    {
      // The TypingCommand will handle the willApplyCommand logic separately in
      // TypingCommand::willAddTypingToOpenCommand.
      return true;
    }

    return CompositeEditCommand::WillApplyCommand();
  }

  void TypingCommand::DoApply() noexcept
  {
    if (EndingSelection().IsNoneOrOrphaned())
    {
      return;
    }

    if (_commandType == Type::DeleteKey)
    {
      if (_commands.empty())
      {
        _openedByBackwardDelete = true;
      }
    }

    switch (_commandType)
    {
      case Type::DeleteSelection:                         DeleteSelection(_smartDelete); return;
      case Type::DeleteKey:                               DeleteKeyPressed(_granularity, _shouldAddToKillRing); return;
      case Type::ForwardDeleteKey:                        ForwardDeleteKeyPressed(_granularity, _shouldAddToKillRing); return;
      case Type::InsertLineBreak:                         InsertLineBreak(); return;
      case Type::InsertParagraphSeparator:                InsertParagraphSeparator(); return;
      case Type::InsertParagraphSeparatorInQuotedContent: InsertParagraphSeparatorInQuotedContent(); return;
      case Type::InsertText:                              InsertText(_textToInsert, _selectInsertedText); return;
    }

    krys_debug_assert(false);
  }

  dom::DOMString TypingCommand::InputEventTypeName() const noexcept
  {
    return dom::DOMString(InputTypeNameForEditingAction(_currentTypingEditAction));
  }

  bool TypingCommand::IsInputMethodComposing() const noexcept
  {
    return IsInputMethodComposingForEditingAction(_currentTypingEditAction);
  }

  bool TypingCommand::IsBeforeInputEventCancelable() const noexcept
  {
    return _currentTypingEditAction != EditAction::TypingInsertPendingComposition
           && _currentTypingEditAction != EditAction::TypingDeletePendingComposition;
  }

  dom::DOMString TypingCommand::InputEventData() const noexcept
  {
    switch (_currentTypingEditAction)
    {
      case EditAction::TypingInsertText:
      case EditAction::TypingInsertPendingComposition:
      case EditAction::TypingInsertFinalComposition:   return _currentTextToInsert;
      case EditAction::InsertReplacement:
        return IsEditingTextAreaOrTextInput() ? _currentTextToInsert : dom::DOMString();
      default: return CompositeEditCommand::InputEventData();
    }
  }

  void TypingCommand::DidApplyCommand() noexcept
  {
    // TypingCommands handle applied editing separately (see TypingCommand::typingAddedToOpenCommand).
    _isHandlingInitialTypingCommand = false;
  }

  bool TypingCommand::WillAddTypingToOpenCommand(Type commandType, TextGranularity granularity,
                                                 utf8_stringview text,
                                                 const Maybe<dom::SimpleRange> &range) noexcept
  {
    _currentTextToInsert = utf8_string(text);
    _currentTypingEditAction =
      EditActionForTypingCommand(commandType, granularity, _compositionType, _isAutocompletion);

    if (!ShouldDeferWillApplyCommandUntilAddingTypingCommand())
    {
      return true;
    }

    if (!range || IsEditingTextAreaOrTextInput())
    {
      return GetDocument().GetEditor().WillApplyEditing(*this,
                                                        CompositeEditCommand::TargetRangesForBindings());
    }

    List<RefPtr<dom::StaticRange>> targetRanges;
    if (auto init = dom::StaticRangeInit::Create(range->Start, range->End); !init.HasException())
    {
      targetRanges.push_back(CreateRef<dom::StaticRange>(init.ReleaseValue()));
    }
    return GetDocument().GetEditor().WillApplyEditing(*this, krys::move(targetRanges));
  }

  void TypingCommand::TypingAddedToOpenCommand(Type commandTypeForAddedTyping) noexcept
  {
    RefPtr<LocalFrame> protectedFrame = ShareRefPtr(GetDocument().Frame());

    UpdatePreservesTypingStyle(commandTypeForAddedTyping);

    GetDocument().GetEditor().AppliedEditing(*this);
  }

  void TypingCommand::InsertText(utf8_stringview text, bool selectInsertedText) noexcept
  {
    // FIXME: Need to implement selectInsertedText for cases where more than one insert is involved.
    // This requires support from insertTextRunWithoutNewlines and insertParagraphSeparator for extending
    // an existing selection; at the moment they can either put the caret after what's inserted or
    // select what's inserted, but there's no way to "extend selection" to include both an old selection
    // that ends just before where we want to insert text and the newly inserted text.
    TypingCommandLineOperation operation(*this, selectInsertedText, text);
    ForEachLineInString(text, operation);
  }

  void TypingCommand::InsertTextRunWithoutNewlines(utf8_stringview text, bool selectInsertedText) noexcept
  {
    if (!WillAddTypingToOpenCommand(Type::InsertText, TextGranularity::CharacterGranularity, text))
    {
      return;
    }

    auto allowPasswordEcho = TriggeringEventIsUntrusted() ? AllowPasswordEcho::No : AllowPasswordEcho::Yes;
    auto rebalanceWhitespaces = _compositionType == TextCompositionType::None
                                  ? InsertTextCommand::RebalanceLeadingAndTrailingWhitespaces
                                  : InsertTextCommand::RebalanceAllWhitespaces;
    auto command = InsertTextCommand::Create(GetDocument(), text, allowPasswordEcho, selectInsertedText,
                                             rebalanceWhitespaces, EditAction::TypingInsertText);

    ApplyCommandToComposite(krys::move(command), EndingSelection());
    TypingAddedToOpenCommand(Type::InsertText);
  }

  void TypingCommand::InsertLineBreak() noexcept
  {
    if (!CanAppendNewLineFeedToSelection(EndingSelection()))
    {
      return;
    }

    if (!WillAddTypingToOpenCommand(Type::InsertLineBreak, TextGranularity::LineGranularity))
    {
      return;
    }

    ApplyCommandToComposite(InsertLineBreakCommand::Create(GetDocument()));
    TypingAddedToOpenCommand(Type::InsertLineBreak);
  }

  void TypingCommand::InsertParagraphSeparator() noexcept
  {
    if (!CanAppendNewLineFeedToSelection(EndingSelection()))
    {
      return;
    }

    if (!WillAddTypingToOpenCommand(Type::InsertParagraphSeparator, TextGranularity::ParagraphGranularity))
    {
      return;
    }

    ApplyCommandToComposite(InsertParagraphSeparatorCommand::Create(GetDocument(), false, false,
                                                                    EditAction::TypingInsertParagraph));
    TypingAddedToOpenCommand(Type::InsertParagraphSeparator);
  }

  void TypingCommand::InsertParagraphSeparatorInQuotedContent() noexcept
  {
    if (!WillAddTypingToOpenCommand(Type::InsertParagraphSeparatorInQuotedContent,
                                    TextGranularity::ParagraphGranularity))
    {
      return;
    }

    // If the selection starts inside a table, just insert the paragraph separator normally
    // Breaking the blockquote would also break apart the table, which is unecessary when inserting a newline
    if (EnclosingNodeOfType(EndingSelection().Start(), &IsTableStructureNode))
    {
      InsertParagraphSeparator();
      return;
    }

    ApplyCommandToComposite(BreakBlockquoteCommand::Create(GetDocument()));
    TypingAddedToOpenCommand(Type::InsertParagraphSeparatorInQuotedContent);
  }

  bool TypingCommand::MakeEditableRootEmpty() noexcept
  {
    RefPtr<dom::Element> root = ShareRefPtr(EndingSelection().RootEditableElement());
    if (!root || !root->FirstChild())
    {
      return false;
    }

    if (root->FirstChild() == root->LastChild() && IsHTMLElementOfType(root->FirstChild(), html::TagName::br))
    {
      // If there is a single child and it could be a placeholder, leave it alone.
      if (root->Renderer() && root->Renderer()->IsRenderBlockFlow())
      {
        return false;
      }
    }

    while (auto *child = root->FirstChild())
    {
      RemoveNode(*child);
    }

    (void)AddBlockPlaceholderIfNeeded(root.get());
    SetEndingSelection(VisibleSelection(dom::FirstPositionInNode(root.get()), Affinity::Downstream,
                                        EndingSelection().GetDirectionality()));

    return true;
  }

  void TypingCommand::DeleteKeyPressed(TextGranularity granularity, bool shouldAddToKillRing) noexcept
  {
    RefPtr<LocalFrame> protectedFrame = ShareRefPtr(GetDocument().Frame());

    VisibleSelection selectionToDelete;
    VisibleSelection selectionAfterUndo;
    bool expandForSpecialElements = false;

    krys_debug_assert(EndingSelection().IsCaretOrRange());

    if (EndingSelection().IsRange())
    {
      selectionToDelete = EndingSelection();
      selectionAfterUndo = selectionToDelete;
      expandForSpecialElements = true;
    }
    else
    {
      // After breaking out of an empty mail blockquote, we still want continue with the deletion
      // so actual content will get deleted, and not just the quote style.
      if (BreakOutOfEmptyMailBlockquotedParagraph())
      {
        TypingAddedToOpenCommand(Type::DeleteKey);
      }

      _smartDelete = false;

      FrameSelection selection;
      selection.SetSelection(EndingSelection());
      (void)selection.Modify(FrameSelection::Alteration::Extend, SelectionDirection::Backward, granularity);
      if (shouldAddToKillRing && selection.IsCaret() && granularity != TextGranularity::CharacterGranularity)
      {
        (void)selection.Modify(FrameSelection::Alteration::Extend, SelectionDirection::Backward,
                               TextGranularity::CharacterGranularity);
      }

      const VisiblePosition visibleStart = EndingSelection().VisibleStart();
      const VisiblePosition previousPosition =
        visibleStart.Previous(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
      auto enclosingTableCell = EnclosingNodeOfType(visibleStart.DeepEquivalent(), &IsTableCell);
      auto enclosingTableCellForPreviousPosition =
        EnclosingNodeOfType(previousPosition.DeepEquivalent(), &IsTableCell);
      if (previousPosition.IsNull() || enclosingTableCell != enclosingTableCellForPreviousPosition)
      {
        // When the caret is at the start of the editable area in an empty list item, break out of the list
        // item.
        if (auto deleteListSelection = ShouldBreakOutOfEmptyListItem(); !deleteListSelection.IsNone())
        {
          if (WillAddTypingToOpenCommand(Type::DeleteKey, granularity, {}, deleteListSelection.FirstRange()))
          {
            (void)BreakOutOfEmptyListItem();
            TypingAddedToOpenCommand(Type::DeleteKey);
          }
          return;
        }
      }
      if (previousPosition.IsNull())
      {
        // When there are no visible positions in the editing root, delete its entire contents.
        // FIXME: Dispatch a `beforeinput` event here and bail if preventDefault() was invoked.
        if (visibleStart.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary).IsNull()
            && MakeEditableRootEmpty())
        {
          TypingAddedToOpenCommand(Type::DeleteKey);
          return;
        }
      }

      // If we have a caret selection at the beginning of a cell, we have nothing to do.
      if (enclosingTableCell
          && visibleStart == VisiblePosition(dom::FirstPositionInNode(enclosingTableCell.get())))
      {
        return;
      }

      // If the caret is at the start of a paragraph after a table, move content into the last table cell.
      if (IsStartOfParagraph(visibleStart)
          && IsFirstPositionAfterTable(
            visibleStart.Previous(EditingBoundaryCrossingRule::CannotCrossEditingBoundary)))
      {
        // Unless the caret is just before a table.  We don't want to move a table into the last table cell.
        if (IsLastPositionBeforeTable(visibleStart))
        {
          return;
        }
        // Extend the selection backward into the last cell, then deletion will handle the move.
        (void)selection.Modify(FrameSelection::Alteration::Extend, SelectionDirection::Backward, granularity);
        // If the caret is just after a table, select the table and don't delete anything.
      }
      else if (auto table = IsFirstPositionAfterTable(visibleStart))
      {
        SetEndingSelection(VisibleSelection(dom::PositionBeforeNode(table.get()), EndingSelection().Start(),
                                            Affinity::Downstream, EndingSelection().GetDirectionality()));
        TypingAddedToOpenCommand(Type::DeleteKey);
        return;
      }

      selectionToDelete = selection.Selection();

      if (granularity == TextGranularity::CharacterGranularity
          && selectionToDelete.End().GetContainerNode() == selectionToDelete.Start().GetContainerNode()
          && selectionToDelete.End().ComputeOffsetInContainerNode()
                 - selectionToDelete.Start().ComputeOffsetInContainerNode()
               > 1)
      {
        // If there are multiple Unicode code points to be deleted, adjust the range to match platform
        // conventions.
        selectionToDelete.SetWithoutValidation(
          selectionToDelete.End(), selectionToDelete.End().Previous(dom::PositionMoveType::BackwardDeletion));
      }

      if (!StartingSelection().IsRange() || selectionToDelete.Base() != StartingSelection().Start())
      {
        selectionAfterUndo = selectionToDelete;
      }
      else
      {
        // It's a little tricky to compute what the starting selection would have been in the original
        // document. We can't let the VisibleSelection class's validation kick in or it'll adjust for us based
        // on the current state of the document and we'll get the wrong result.
        selectionAfterUndo.SetWithoutValidation(StartingSelection().End(), selectionToDelete.Extent());
      }
    }

    krys_debug_assert(!selectionToDelete.IsNone());
    if (selectionToDelete.IsNone())
    {
      return;
    }

    if (selectionToDelete.IsCaret() || !GetDocument().Selection().ShouldDeleteSelection(selectionToDelete))
    {
      return;
    }

    if (!WillAddTypingToOpenCommand(Type::DeleteKey, granularity, {}, selectionToDelete.FirstRange()))
    {
      return;
    }

    // Make undo select everything that has been deleted, unless an undo will undo more than just this
    // deletion.
    // FIXME: This behaves like TextEdit except for the case where you open with text insertion and then
    // delete more text than you insert.  In that case all of the text that was around originally should be
    // selected.
    if (_openedByBackwardDelete)
    {
      SetStartingSelection(selectionAfterUndo);
    }
    CompositeEditCommand::DeleteSelection(selectionToDelete, _smartDelete, /* mergeBlocksAfterDelete*/ true,
                                          /* replace*/ false, expandForSpecialElements,
                                          /*sanitizeMarkup*/ true);
    SetSmartDelete(false);
    TypingAddedToOpenCommand(Type::DeleteKey);
  }

  void TypingCommand::ForwardDeleteKeyPressed(TextGranularity granularity, bool shouldAddToKillRing) noexcept
  {
    RefPtr<LocalFrame> protectedFrame = ShareRefPtr(GetDocument().Frame());

    VisibleSelection selectionToDelete;
    VisibleSelection selectionAfterUndo;
    bool expandForSpecialElements = false;

    krys_debug_assert(EndingSelection().IsCaretOrRange());

    if (EndingSelection().IsRange())
    {
      selectionToDelete = EndingSelection();
      selectionAfterUndo = selectionToDelete;
      expandForSpecialElements = true;
    }
    else
    {
      _smartDelete = false;

      // Handle delete at beginning-of-block case.
      // Do nothing in the case that the caret is at the start of a
      // root editable element or at the start of a document.
      FrameSelection selection;
      selection.SetSelection(EndingSelection());
      (void)selection.Modify(FrameSelection::Alteration::Extend, SelectionDirection::Forward, granularity);
      if (selection.IsNone())
      {
        return;
      }
      if (shouldAddToKillRing && selection.IsCaret() && granularity != TextGranularity::CharacterGranularity)
      {
        (void)selection.Modify(FrameSelection::Alteration::Extend, SelectionDirection::Forward,
                               TextGranularity::CharacterGranularity);
      }

      dom::Position downstreamEnd = EndingSelection().End().Downstream();
      VisiblePosition visibleEnd = EndingSelection().VisibleEnd();
      auto enclosingTableCell = EnclosingNodeOfType(visibleEnd.DeepEquivalent(), &IsTableCell);
      if (enclosingTableCell
          && visibleEnd == VisiblePosition(dom::LastPositionInNode(enclosingTableCell.get())))
      {
        return;
      }
      if (visibleEnd == EndOfParagraph(visibleEnd))
      {
        downstreamEnd = visibleEnd.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary)
                          .DeepEquivalent()
                          .Downstream();
      }
      // When deleting tables: Select the table first, then perform the deletion
      if (downstreamEnd.GetContainerNode() && downstreamEnd.GetContainerNode()->Renderer()
          && downstreamEnd.GetContainerNode()->Renderer()->IsRenderTable()
          && downstreamEnd.ComputeOffsetInContainerNode()
               <= CaretMinOffset(*downstreamEnd.GetContainerNode()))
      {
        SetEndingSelection(VisibleSelection(EndingSelection().End(),
                                            dom::PositionAfterNode(downstreamEnd.GetContainerNode()),
                                            Affinity::Downstream, EndingSelection().GetDirectionality()));
        TypingAddedToOpenCommand(Type::ForwardDeleteKey);
        return;
      }

      // deleting to end of paragraph when at end of paragraph needs to merge the next paragraph (if any)
      if (granularity == TextGranularity::ParagraphBoundary && selection.Selection().IsCaret()
          && IsEndOfParagraph(selection.Selection().VisibleEnd()))
      {
        (void)selection.Modify(FrameSelection::Alteration::Extend, SelectionDirection::Forward,
                               TextGranularity::CharacterGranularity);
      }

      selectionToDelete = selection.Selection();
      if (!StartingSelection().IsRange() || selectionToDelete.Base() != StartingSelection().Start())
      {
        selectionAfterUndo = selectionToDelete;
      }
      else
      {
        // It's a little tricky to compute what the starting selection would have been in the original
        // document. We can't let the VisibleSelection class's validation kick in or it'll adjust for us based
        // on the current state of the document and we'll get the wrong result.
        dom::Position extent = StartingSelection().End();
        if (extent.GetContainerNode() != selectionToDelete.End().GetContainerNode())
        {
          extent = selectionToDelete.Extent();
        }
        else
        {
          int32 extraCharacters;
          if (selectionToDelete.Start().GetContainerNode() == selectionToDelete.End().GetContainerNode())
          {
            extraCharacters = selectionToDelete.End().ComputeOffsetInContainerNode()
                              - selectionToDelete.Start().ComputeOffsetInContainerNode();
          }
          else
          {
            extraCharacters = selectionToDelete.End().ComputeOffsetInContainerNode();
          }
          extent = dom::Position(ShareRefPtr(extent.GetContainerNode()),
                                 static_cast<uint32>(extent.ComputeOffsetInContainerNode() + extraCharacters),
                                 dom::Position::AnchorType::PositionIsOffsetInAnchor);
        }
        selectionAfterUndo.SetWithoutValidation(StartingSelection().Start(), extent);
      }
    }

    krys_debug_assert(!selectionToDelete.IsNone());
    if (selectionToDelete.IsNone())
    {
      return;
    }

    if (selectionToDelete.IsCaret() || !GetDocument().Selection().ShouldDeleteSelection(selectionToDelete))
    {
      return;
    }

    if (!WillAddTypingToOpenCommand(Type::ForwardDeleteKey, granularity, {}, selectionToDelete.FirstRange()))
    {
      return;
    }

    // make undo select what was deleted
    SetStartingSelection(selectionAfterUndo);
    CompositeEditCommand::DeleteSelection(selectionToDelete, _smartDelete, /* mergeBlocksAfterDelete*/ true,
                                          /* replace*/ false, expandForSpecialElements,
                                          /*sanitizeMarkup*/ true);
    SetSmartDelete(false);
    TypingAddedToOpenCommand(Type::ForwardDeleteKey);
  }

  void TypingCommand::DeleteSelection(bool smartDelete) noexcept
  {
    if (!WillAddTypingToOpenCommand(Type::DeleteSelection, TextGranularity::CharacterGranularity))
    {
      return;
    }

    CompositeEditCommand::DeleteSelection(smartDelete);
    TypingAddedToOpenCommand(Type::DeleteSelection);
  }

  void TypingCommand::UpdatePreservesTypingStyle(Type commandType) noexcept
  {
    switch (commandType)
    {
      case Type::DeleteSelection:
      case Type::DeleteKey:
      case Type::ForwardDeleteKey:
      case Type::InsertParagraphSeparator:
      case Type::InsertLineBreak:                         _preservesTypingStyle = true; return;
      case Type::InsertParagraphSeparatorInQuotedContent:
      case Type::InsertText:                              _preservesTypingStyle = false; return;
    }
    krys_debug_assert(false);
    _preservesTypingStyle = false;
  }

  bool TypingCommand::IsTypingCommand() const noexcept
  {
    return true;
  }
}

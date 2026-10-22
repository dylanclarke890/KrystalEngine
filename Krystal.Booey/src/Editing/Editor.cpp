#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/ClipboardEvent.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/InputEvent.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/TextEvent.hpp"
#include "Krystal.Booey/Editing/CompositeEditCommand.hpp"
#include "Krystal.Booey/Editing/DeleteSelectionCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/Markup.hpp"
#include "Krystal.Booey/Editing/ReplaceSelectionCommand.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/Editing/TypingCommand.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Page/EditorClient.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/Pasteboard.hpp"
#include "Krystal.Booey/Platform/ScrollAlignment.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD utf8_stringview EventNameForClipboardEvent(ClipboardEventKind kind) noexcept
    {
      switch (kind)
      {
        case ClipboardEventKind::Copy:
        case ClipboardEventKind::CopyFont:         return EventNames::Copy;
        case ClipboardEventKind::Cut:              return EventNames::Cut;
        case ClipboardEventKind::Paste:
        case ClipboardEventKind::PasteAsPlainText:
        case ClipboardEventKind::PasteAsQuotation:
        case ClipboardEventKind::PasteFont:        return EventNames::Paste;
        case ClipboardEventKind::BeforeCopy:       return EventNames::BeforeCopy;
        case ClipboardEventKind::BeforeCut:        return EventNames::BeforeCut;
        case ClipboardEventKind::BeforePaste:      return EventNames::BeforePaste;
      }
      krys_debug_assert(false);
      return {};
    }

    // FIXME: The data transfer of pastes and drops, with DataTransfer.
    bool DispatchBeforeInputEvent(dom::Element &element, utf8_stringview inputType,
                                  dom::IsInputMethodComposing isInputMethodComposing,
                                  Maybe<dom::DOMString> data = null,
                                  const List<RefPtr<dom::StaticRange>> &targetRanges = {},
                                  bool cancelable = true) noexcept
    {
      auto event = dom::InputEvent::Create(dom::DOMStringAtom(EventNames::BeforeInput), inputType, cancelable,
                                           krys::move(data), targetRanges, 0, isInputMethodComposing);
      (void)dom::EventDispatcher::DispatchToTarget(*event, &element);
      return !event->DefaultPrevented();
    }

    // FIXME: Typing commands that are applied again fire `input` from within the command, so it's dispatched
    // to a scoped event queue rather than straight away, with a scoped event queue.
    void DispatchInputEvent(dom::Element &element, utf8_stringview inputType,
                            dom::IsInputMethodComposing isInputMethodComposing,
                            Maybe<dom::DOMString> data = null,
                            const List<RefPtr<dom::StaticRange>> &targetRanges = {}) noexcept
    {
      auto event = dom::InputEvent::Create(dom::DOMStringAtom(EventNames::Input), inputType, false,
                                           krys::move(data), targetRanges, 0, isInputMethodComposing);
      (void)dom::EventDispatcher::DispatchToTarget(*event, &element);
    }

    bool DispatchBeforeInputEvents(RefPtr<dom::Element> startRoot, RefPtr<dom::Element> endRoot,
                                   utf8_stringview inputTypeName,
                                   dom::IsInputMethodComposing isInputMethodComposing,
                                   const Maybe<dom::DOMString> &data = null,
                                   const List<RefPtr<dom::StaticRange>> &targetRanges = {},
                                   bool cancelable = true) noexcept
    {
      bool continueWithDefaultBehavior = true;
      if (startRoot)
      {
        continueWithDefaultBehavior &= DispatchBeforeInputEvent(
          *startRoot, inputTypeName, isInputMethodComposing, data, targetRanges, cancelable);
      }
      if (endRoot && endRoot != startRoot)
      {
        continueWithDefaultBehavior &= DispatchBeforeInputEvent(
          *endRoot, inputTypeName, isInputMethodComposing, data, targetRanges, cancelable);
      }
      return continueWithDefaultBehavior;
    }

    void DispatchInputEvents(RefPtr<dom::Element> startRoot, RefPtr<dom::Element> endRoot,
                             utf8_stringview inputTypeName,
                             dom::IsInputMethodComposing isInputMethodComposing,
                             const Maybe<dom::DOMString> &data = null,
                             const List<RefPtr<dom::StaticRange>> &targetRanges = {}) noexcept
    {
      if (startRoot)
      {
        DispatchInputEvent(*startRoot, inputTypeName, isInputMethodComposing, data, targetRanges);
      }
      if (endRoot && endRoot != startRoot)
      {
        DispatchInputEvent(*endRoot, inputTypeName, isInputMethodComposing, data, targetRanges);
      }
    }

    KRYS_NODISCARD Maybe<dom::DOMString> InputEventData(const CompositeEditCommand &command) noexcept
    {
      auto data = command.InputEventData();
      if (data.empty())
      {
        return null;
      }
      return data;
    }
  }

  Editor::Editor(dom::Document &document) noexcept : _document(document)
  {
  }

  Editor::~Editor() noexcept = default;

  EditorClient *Editor::Client() const noexcept
  {
    if (auto *page = _document.GetPage())
    {
      return &page->GetEditorClient();
    }
    return nullptr;
  }

  // FIXME: Input methods (handleInputMethodKeydown, didDispatchInputMethodKeydown), with text input.
  bool Editor::HandleKeyboardEvent(dom::KeyboardEvent &event) noexcept
  {
    if (auto *client = Client())
    {
      return client->HandleKeyboardEvent(event);
    }
    return false;
  }

  // Returns whether caller should continue with "the default processing", which is the same as
  // the event handler NOT setting the return value to false
  // FIXME: The clipboard data the event's listeners write, committed to the pasteboard when they cancel a
  // copy or cut, with DataTransfer.
  bool Editor::DispatchClipboardEvent(RefPtr<dom::Element> &&target, ClipboardEventKind kind) noexcept
  {
    // FIXME: Move the target selection code here.
    if (!target)
    {
      return true;
    }

    auto event = dom::ClipboardEvent::Create(dom::DOMStringAtom(EventNameForClipboardEvent(kind)));

    (void)dom::EventDispatcher::DispatchToTarget(*event, target.get());
    bool noDefaultProcessing = event->DefaultPrevented();

    return !noDefaultProcessing;
  }

  // WinIE uses onbeforecut and onbeforepaste to enables the cut and paste menu items.  They
  // also send onbeforecopy, apparently for symmetry, but it doesn't affect the menu items.
  // We need to use onbeforecopy as a real menu enabler because we allow elements that are not
  // normally selectable to implement copy/paste (like divs, or a document body).

  // FIXME: Password fields, with form controls.
  bool Editor::CanDHTMLCut() noexcept
  {
    return !DispatchClipboardEvent(FindEventTargetFromSelection(), ClipboardEventKind::BeforeCut);
  }

  // FIXME: Password fields, with form controls.
  bool Editor::CanDHTMLCopy() noexcept
  {
    return !DispatchClipboardEvent(FindEventTargetFromSelection(), ClipboardEventKind::BeforeCopy);
  }

  bool Editor::CanDHTMLPaste() noexcept
  {
    return !DispatchClipboardEvent(FindEventTargetFromSelection(), ClipboardEventKind::BeforePaste);
  }

  bool Editor::CanCut() const noexcept
  {
    return CanCopy() && CanDelete();
  }

  // FIXME: Image documents and password fields, with them.
  bool Editor::CanCopy() const noexcept
  {
    const VisibleSelection &selection = _document.Selection().Selection();
    return selection.IsRange() || !IsEditablePosition(selection.Start());
  }

  bool Editor::CanPaste() const noexcept
  {
    return CanEdit();
  }

  // FIXME: Password fields, with form controls.
  bool Editor::TryDHTMLCopy() noexcept
  {
    return !DispatchClipboardEvent(FindEventTargetFromSelection(), ClipboardEventKind::Copy);
  }

  // FIXME: Password fields, with form controls.
  bool Editor::TryDHTMLCut() noexcept
  {
    return !DispatchClipboardEvent(FindEventTargetFromSelection(), ClipboardEventKind::Cut);
  }

  RefPtr<dom::Element> Editor::FindEventTargetFrom(const VisibleSelection &selection) const noexcept
  {
    RefPtr<dom::Element> target {selection.Start().AnchorElementAncestor()};
    if (!target)
    {
      target = ShareRefPtr<dom::Element>(_document.BodyOrFrameset());
    }

    return target;
  }

  RefPtr<dom::Element> Editor::FindEventTargetFromSelection() const noexcept
  {
    return FindEventTargetFrom(_document.Selection().Selection());
  }

  // FIXME: Beeping when there's nothing to cut (the system sound manager), once the host can.
  void Editor::Cut(FromMenuOrKeyBinding fromMenuOrKeyBinding) noexcept
  {
    bool oldCopying =
      std::exchange(_copyingFromMenuOrKeyBinding, fromMenuOrKeyBinding == FromMenuOrKeyBinding::Yes);
    if (TryDHTMLCut())
    {
      // DHTML did the whole operation
      _copyingFromMenuOrKeyBinding = oldCopying;
      return;
    }
    if (CanCut())
    {
      PerformCutOrCopy(EditorActionSpecifier::CutAction);
    }
    _copyingFromMenuOrKeyBinding = oldCopying;
  }

  // FIXME: Beeping when there's nothing to copy (the system sound manager), once the host can.
  void Editor::Copy(FromMenuOrKeyBinding fromMenuOrKeyBinding) noexcept
  {
    bool oldCopying =
      std::exchange(_copyingFromMenuOrKeyBinding, fromMenuOrKeyBinding == FromMenuOrKeyBinding::Yes);
    if (TryDHTMLCopy())
    {
      // DHTML did the whole operation
      _copyingFromMenuOrKeyBinding = oldCopying;
      return;
    }
    if (CanCopy())
    {
      PerformCutOrCopy(EditorActionSpecifier::CopyAction);
    }
    _copyingFromMenuOrKeyBinding = oldCopying;
  }

  void Editor::Paste(FromMenuOrKeyBinding fromMenuOrKeyBinding) noexcept
  {
    Paste(*Pasteboard::CreateForCopyAndPaste(), fromMenuOrKeyBinding);
  }

  // FIXME: Spelling markers (updateMarkersForWordsAffectedByEditing), with text checking.
  void Editor::Paste(Pasteboard &pasteboard, FromMenuOrKeyBinding fromMenuOrKeyBinding) noexcept
  {
    bool oldPasting =
      std::exchange(_pastingFromMenuOrKeyBinding, fromMenuOrKeyBinding == FromMenuOrKeyBinding::Yes);

    if (DispatchClipboardEvent(FindEventTargetFromSelection(), ClipboardEventKind::Paste) && CanPaste())
    {
      if (_document.Selection().Selection().IsContentRichlyEditable())
      {
        PasteWithPasteboard(&pasteboard, {PasteOption::AllowPlainText});
      }
      else
      {
        PasteAsPlainTextWithPasteboard(pasteboard);
      }
    }
    _pastingFromMenuOrKeyBinding = oldPasting;
  }

  // FIXME: Spelling markers (updateMarkersForWordsAffectedByEditing), with text checking.
  void Editor::PasteAsPlainText(FromMenuOrKeyBinding fromMenuOrKeyBinding) noexcept
  {
    bool oldPasting =
      std::exchange(_pastingFromMenuOrKeyBinding, fromMenuOrKeyBinding == FromMenuOrKeyBinding::Yes);

    if (DispatchClipboardEvent(FindEventTargetFromSelection(), ClipboardEventKind::PasteAsPlainText)
        && CanPaste())
    {
      PasteAsPlainTextWithPasteboard(*Pasteboard::CreateForCopyAndPaste());
    }
    _pastingFromMenuOrKeyBinding = oldPasting;
  }

  void Editor::PasteAsPlainText(utf8_stringview pastingText, bool smartReplace) noexcept
  {
    RefPtr<dom::Element> target = FindEventTargetFromSelection();
    if (!target)
    {
      return;
    }
    auto event = dom::TextEvent::CreateForPlainTextPaste(pastingText, smartReplace);
    (void)dom::EventDispatcher::DispatchToTarget(*event, target.get());
    if (!event->DefaultPrevented())
    {
      (void)HandleTextEvent(*event);
    }
  }

  // FIXME: The edit action of the paste (the event's input type), with drag and drop and removing
  // backgrounds.
  void Editor::PasteAsFragment(Ref<dom::DocumentFragment> &&pastingFragment, bool smartReplace,
                               bool matchStyle, MailBlockquoteHandling respectsMailBlockquote,
                               EditAction) noexcept
  {
    RefPtr<dom::Element> target = FindEventTargetFromSelection();
    if (!target)
    {
      return;
    }
    auto event =
      dom::TextEvent::CreateForFragmentPaste(krys::move(pastingFragment), dom::TextEventInputType::Paste,
                                             smartReplace, matchStyle, respectsMailBlockquote);
    (void)dom::EventDispatcher::DispatchToTarget(*event, target.get());
    if (!event->DefaultPrevented())
    {
      (void)HandleTextEvent(*event);
    }
  }

  void Editor::PasteAsPlainTextWithPasteboard(Pasteboard &pasteboard) noexcept
  {
    utf8_string text = ReadPlainTextFromPasteboard(pasteboard);
    if (ShouldInsertText(text, SelectedRange()))
    {
      PasteAsPlainText(text, CanSmartReplaceWithPasteboard(pasteboard));
    }
  }

  // FIXME: Quoting the fragment (quoteFragmentForPasting), with mail blockquotes.
  void Editor::PasteWithPasteboard(Pasteboard *pasteboard, EnumSet<PasteOption> options) noexcept
  {
    auto range = SelectedRange();
    if (!range)
    {
      return;
    }
    bool chosePlainText = false;
    auto fragment =
      pasteboard->ReadDocumentFragment(*range, options.Contains(PasteOption::AllowPlainText), chosePlainText);
    if (fragment && ShouldInsertFragment(*fragment, range))
    {
      PasteAsFragment(ShareRef(*fragment), CanSmartReplaceWithPasteboard(*pasteboard), chosePlainText,
                      options.Contains(PasteOption::IgnoreMailBlockquote)
                        ? MailBlockquoteHandling::IgnoreBlockquote
                        : MailBlockquoteHandling::RespectBlockquote);
    }
  }

  utf8_string Editor::ReadPlainTextFromPasteboard(Pasteboard &pasteboard) noexcept
  {
    PasteboardPlainText text;
    pasteboard.Read(text);
    return krys::move(text.Text);
  }

  bool Editor::CanSmartReplaceWithPasteboard(Pasteboard &pasteboard) noexcept
  {
    return SmartInsertDeleteEnabled() && pasteboard.CanSmartReplaceContents();
  }

  // FIXME: Asking the editor client (shouldInsertText and shouldInsertNode), with the editor client's
  // delegate callbacks.
  bool Editor::ShouldInsertFragment(dom::DocumentFragment &fragment,
                                    const Maybe<dom::SimpleRange> &replacingDOMRange) const noexcept
  {
    if (!Client())
    {
      return false;
    }
    if (auto *child = fragment.FirstChild();
        child && fragment.LastChild() == child && Is<dom::CharacterData>(*child))
    {
      return ShouldInsertText(Downcast<dom::CharacterData>(*child).Data(), replacingDOMRange);
    }
    return true;
  }

  // FIXME: Text form controls, image documents, image overlays and writing the selection as web content
  // (writeSelectionToPasteboard), with form controls, images and DataTransfer; the editor client's
  // notifications around writing the selection; spelling markers (updateMarkersForWordsAffectedByEditing),
  // with text checking; and accessibility's notification of the cut text, with accessibility.
  void Editor::PerformCutOrCopy(EditorActionSpecifier action) noexcept
  {
    auto selection = SelectedRange();
    if (action == EditorActionSpecifier::CutAction && !ShouldDeleteRange(selection))
    {
      return;
    }

    Pasteboard::CreateForCopyAndPaste()->WritePlainText(
      SelectedTextForDataTransfer(),
      CanSmartCopyOrDelete() ? Pasteboard::CanSmartReplace : Pasteboard::CannotSmartReplace);

    if (action == EditorActionSpecifier::CutAction)
    {
      DeleteSelectionWithSmartDelete(CanSmartCopyOrDelete(), EditAction::Cut);
    }
  }

  EditingBehavior Editor::Behavior() const noexcept
  {
    return EditingBehavior(_document.GetSettings().EditingBehaviorType);
  }

  // FIXME: Writing Tools suppressing editing, and stopping scrolling as content grows while typing.
  bool Editor::WillApplyEditing(CompositeEditCommand &command,
                                List<RefPtr<dom::StaticRange>> &&targetRanges) noexcept
  {
    if (!command.ShouldDispatchInputEvents())
    {
      return true;
    }

    RefPtr<EditCommandComposition> composition = ShareRefPtr(command.Composition());
    if (!composition)
    {
      return true;
    }

    auto isInputMethodComposing =
      command.IsInputMethodComposing() ? dom::IsInputMethodComposing::Yes : dom::IsInputMethodComposing::No;
    return DispatchBeforeInputEvents(
      ShareRefPtr(composition->StartingRootEditableElement()),
      ShareRefPtr(composition->EndingRootEditableElement()), command.InputEventTypeName(),
      isInputMethodComposing, InputEventData(command), targetRanges, command.IsBeforeInputEventCancelable());
  }

  // FIXME: Notifying text controls of the change (notifyTextFromControls), with form controls; updating the
  // editor's UI, with menus; and autocorrection's response (respondToAppliedEditing), with text checking.
  void Editor::AppliedEditing(CompositeEditCommand &command) noexcept
  {
    _document.UpdateLayout();

    krys_debug_assert(command.Composition());
    Ref<EditCommandComposition> composition = ShareRef(*command.Composition());
    VisibleSelection newSelection(command.EndingSelection());

    if (command.IsTopLevelCommand())
    {
      // Don't clear the typing style with this selection change. We do those things elsewhere if necessary.
      EnumSet<FrameSelection::SetSelectionOption> options;
      if (command.IsDictationCommand())
      {
        options.Add(FrameSelection::SetSelectionOption::DictationTriggered);
      }

      ChangeSelectionAfterCommand(newSelection, options);
    }

    auto isInputMethodComposing =
      command.IsInputMethodComposing() ? dom::IsInputMethodComposing::Yes : dom::IsInputMethodComposing::No;
    if (command.ShouldDispatchInputEvents())
    {
      DispatchInputEvents(ShareRefPtr(composition->StartingRootEditableElement()),
                          ShareRefPtr(composition->EndingRootEditableElement()), command.InputEventTypeName(),
                          isInputMethodComposing, InputEventData(command));
    }

    if (command.IsTopLevelCommand())
    {
      if (!command.PreservesTypingStyle())
      {
        _document.Selection().ClearTypingStyle();
      }

      // Command will be equal to last edit command only in the case of typing
      if (_lastEditCommand.get() == &command)
      {
        krys_debug_assert(command.IsTypingCommand());
      }
      else
      {
        // Only register a new undo command if the command passed in is
        // different from the last command
        _lastEditCommand = ShareRefPtr(&command);
        if (auto *client = Client())
        {
          client->RegisterUndoStep(*_lastEditCommand->EnsureComposition());
        }
      }
      RespondToChangedContents(newSelection);
    }
  }

  bool Editor::WillUnapplyEditing(const EditCommandComposition &composition) const noexcept
  {
    TypingCommand::CloseTyping(_document);
    return DispatchBeforeInputEvents(ShareRefPtr(composition.StartingRootEditableElement()),
                                     ShareRefPtr(composition.EndingRootEditableElement()), u8"historyUndo",
                                     dom::IsInputMethodComposing::No);
  }

  void Editor::UnappliedEditing(EditCommandComposition &composition) noexcept
  {
    _document.UpdateLayout();

    VisibleSelection newSelection(composition.StartingSelection());
    ChangeSelectionAfterCommand(newSelection, FrameSelection::DefaultSetSelectionOptions());
    DispatchInputEvents(ShareRefPtr(composition.StartingRootEditableElement()),
                        ShareRefPtr(composition.EndingRootEditableElement()), u8"historyUndo",
                        dom::IsInputMethodComposing::No);

    _lastEditCommand = nullptr;
    if (auto *client = Client())
    {
      client->RegisterRedoStep(composition);
    }
    RespondToChangedContents(newSelection);
  }

  bool Editor::WillReapplyEditing(const EditCommandComposition &composition) const noexcept
  {
    return DispatchBeforeInputEvents(ShareRefPtr(composition.StartingRootEditableElement()),
                                     ShareRefPtr(composition.EndingRootEditableElement()), u8"historyRedo",
                                     dom::IsInputMethodComposing::No);
  }

  void Editor::ReappliedEditing(EditCommandComposition &composition) noexcept
  {
    _document.UpdateLayout();

    VisibleSelection newSelection(composition.EndingSelection());
    ChangeSelectionAfterCommand(newSelection, FrameSelection::DefaultSetSelectionOptions());
    DispatchInputEvents(ShareRefPtr(composition.StartingRootEditableElement()),
                        ShareRefPtr(composition.EndingRootEditableElement()), u8"historyRedo",
                        dom::IsInputMethodComposing::No);

    _lastEditCommand = nullptr;
    if (auto *client = Client())
    {
      client->RegisterUndoStep(composition);
    }
    RespondToChangedContents(newSelection);
  }

  bool Editor::CanUndo() const noexcept
  {
    auto *client = Client();
    return client && client->CanUndo();
  }

  void Editor::Undo() noexcept
  {
    if (auto *client = Client())
    {
      client->Undo();
    }
  }

  bool Editor::CanRedo() const noexcept
  {
    auto *client = Client();
    return client && client->CanRedo();
  }

  void Editor::Redo() noexcept
  {
    if (auto *client = Client())
    {
      client->Redo();
    }
  }

  CompositeEditCommand *Editor::LastEditCommand() const noexcept
  {
    return _lastEditCommand.get();
  }

  // FIXME: Accessibility's notification of the change, with accessibility; and updating the spelling and
  // grammar markers of the words the edit touched, with text checking.
  void Editor::RespondToChangedContents(const VisibleSelection &) noexcept
  {
    if (auto *client = Client())
    {
      client->RespondToChangedContents();
    }
  }

  void Editor::ChangeSelectionAfterCommand(const VisibleSelection &newSelection,
                                           EnumSet<FrameSelectionSetSelectionOption> options) noexcept
  {
    if (newSelection.IsOrphan() || newSelection.GetDocument() != &_document)
    {
      return;
    }

    // If there is no selection change, don't bother sending shouldChangeSelection, but still call
    // setSelection, because there is work that it must do in this situation. The old selection can be invalid
    // here and calling shouldChangeSelection can produce some strange calls. See <rdar://problem/5729315>
    // Some shouldChangeSelectedDOMRange contain Ranges for selections that are no longer valid
    bool selectionDidNotChangeDOMPosition = newSelection == _document.Selection().Selection();
    if (selectionDidNotChangeDOMPosition || _document.Selection().ShouldChangeSelection(newSelection))
    {
      _document.Selection().SetSelection(newSelection, options);
    }

    krys_debug_assert(!_document.Selection().Selection().IsOrphan());

    // FIXME: Telling the client the selection changed when the edit moved it visually without moving its DOM
    // position (respondToChangedSelection), with the rest of the editor client.
  }

  bool Editor::CanEdit() const noexcept
  {
    return _document.Selection().Selection().RootEditableElement();
  }

  bool Editor::CanEditRichly() const noexcept
  {
    return _document.Selection().Selection().IsContentRichlyEditable();
  }

  bool Editor::CanDelete() const noexcept
  {
    const auto &selection = _document.Selection().Selection();
    return selection.IsRange() && selection.RootEditableElement();
  }

  bool Editor::CanDeleteRange(const dom::SimpleRange &range) const noexcept
  {
    if (!range.Start.Container->HasEditableStyle() || !range.End.Container->HasEditableStyle())
    {
      return false;
    }

    if (range.Collapsed())
    {
      // FIXME: We sometimes allow deletions at the start of editable roots, like when the caret is in an
      // empty list item.
      auto previous = VisiblePosition {dom::MakeDeprecatedLegacyPosition(range.Start)}.Previous();
      if (previous.IsNull()
          || previous.DeepEquivalent().DeprecatedNode()->RootEditableElement()
               != range.Start.Container->RootEditableElement())
      {
        return false;
      }
    }

    return true;
  }

  // FIXME: Asking the editor client whether to delete the range (shouldDeleteRange), with the editing
  // delegates.
  bool Editor::ShouldDeleteRange(const Maybe<dom::SimpleRange> &range) const noexcept
  {
    return range && !range->Collapsed() && CanDeleteRange(*range) && Client();
  }

  bool Editor::ShouldSmartDelete() noexcept
  {
    if (Behavior().ShouldAlwaysSmartDelete())
    {
      return true;
    }
    return _document.Selection().Granularity() == TextGranularity::WordGranularity;
  }

  bool Editor::SmartInsertDeleteEnabled() noexcept
  {
    auto *client = Client();
    return client && client->SmartInsertDeleteEnabled();
  }

  bool Editor::CanSmartCopyOrDelete() noexcept
  {
    return SmartInsertDeleteEnabled() && ShouldSmartDelete();
  }

  Maybe<dom::SimpleRange> Editor::SelectedRange() noexcept
  {
    return _document.Selection().Selection().ToNormalizedRange();
  }

  // FIXME: Asking the editor client whether to insert the text (shouldInsertText), with the editing
  // delegates.
  bool Editor::ShouldInsertText(utf8_stringview, const Maybe<dom::SimpleRange> &) const noexcept
  {
    return Client();
  }

  bool Editor::InsertText(utf8_stringview text, dom::Event *triggeringEvent,
                          dom::TextEventInputType inputType) noexcept
  {
    auto *frame = _document.Frame();
    return frame && frame->GetEventHandler().HandleTextInputEvent(text, triggeringEvent, inputType);
  }

  bool Editor::InsertText(utf8_stringview text, dom::Event *triggeringEvent) noexcept
  {
    return InsertText(text, triggeringEvent, dom::TextEventInputType::Keyboard);
  }

  // FIXME: Dictation (DictationCommand), with dictation; autocorrection before and after the text, with text
  // checking; and revealing the selection when the client asks (shouldRevealCurrentSelectionAfterInsertion),
  // with the rest of the editor client.
  bool Editor::InsertTextWithoutSendingTextEvent(utf8_stringview text, bool selectInsertedText,
                                                 dom::TextEvent *triggeringEvent) noexcept
  {
    if (text.empty())
    {
      return false;
    }

    VisibleSelection selection = SelectionForCommand(triggeringEvent);
    if (!selection.IsContentEditable())
    {
      return false;
    }

    if (!ShouldInsertText(text, selection.ToNormalizedRange()))
    {
      return true;
    }

    // Get the selection to use for the event that triggered this insertText.
    // If the event handler changed the selection, we may want to use a different selection
    // that is contained in the event target.
    selection = SelectionForCommand(triggeringEvent);
    if (selection.IsContentEditable())
    {
      if (auto *selectionStart = selection.Start().DeprecatedNode())
      {
        Ref<dom::Document> document = ShareRef(selectionStart->NodeDocument());

        // Insert the text
        EnumSet<TypingCommand::Option> options {TypingCommand::Option::RetainAutocorrectionIndicator};
        if (selectInsertedText)
        {
          options.Add(TypingCommand::Option::SelectInsertedText);
        }
        if (triggeringEvent && triggeringEvent->IsAutocompletion())
        {
          options.Add(TypingCommand::Option::IsAutocompletion);
        }
        auto compositionType = triggeringEvent && triggeringEvent->IsComposition()
                                 ? TypingCommand::TextCompositionType::Final
                                 : TypingCommand::TextCompositionType::None;
        TypingCommand::InsertText(*document, text, triggeringEvent, selection, options, compositionType);
      }
    }

    return true;
  }

  bool Editor::InsertLineBreak() noexcept
  {
    if (!CanEdit())
    {
      return false;
    }

    if (!ShouldInsertText(u8"\n", _document.Selection().Selection().ToNormalizedRange()))
    {
      return true;
    }

    VisiblePosition caret = _document.Selection().Selection().VisibleStart();
    bool alignToEdge = IsEndOfEditableOrNonEditableContent(caret);
    TypingCommand::InsertLineBreak(_document, {});
    RevealSelectionAfterEditingOperation(alignToEdge ? ScrollAlignment::AlignToEdgeIfNeeded
                                                     : ScrollAlignment::AlignCenterIfNeeded,
                                         RevealExtentOption::DoNotRevealExtent);

    return true;
  }

  bool Editor::InsertParagraphSeparator() noexcept
  {
    if (!CanEdit())
    {
      return false;
    }

    if (!CanEditRichly())
    {
      return InsertLineBreak();
    }

    if (!ShouldInsertText(u8"\n", _document.Selection().Selection().ToNormalizedRange()))
    {
      return true;
    }

    VisiblePosition caret = _document.Selection().Selection().VisibleStart();
    bool alignToEdge = IsEndOfEditableOrNonEditableContent(caret);
    TypingCommand::InsertParagraphSeparator(_document, {});
    RevealSelectionAfterEditingOperation(alignToEdge ? ScrollAlignment::AlignToEdgeIfNeeded
                                                     : ScrollAlignment::AlignCenterIfNeeded,
                                         RevealExtentOption::DoNotRevealExtent);

    return true;
  }

  bool Editor::InsertParagraphSeparatorInQuotedContent() noexcept
  {
    // FIXME: Why is this missing calls to canEdit, canEditRichly, etc.?
    TypingCommand::InsertParagraphSeparatorInQuotedContent(_document);
    RevealSelectionAfterEditingOperation();
    return true;
  }

  // FIXME: Drops, which the drag controller handles, with drag and drop.
  bool Editor::HandleTextEvent(dom::TextEvent &event) noexcept
  {
    // Default event handling for Drag and Drop will be handled by DragController
    // so we leave the event for it.
    if (event.IsDrop())
    {
      return false;
    }

    if (event.IsPaste() || event.IsRemoveBackground())
    {
      auto action = event.IsRemoveBackground() ? EditAction::RemoveBackground : EditAction::Paste;
      if (event.PastingFragment())
      {
        ReplaceSelectionWithFragment(*event.PastingFragment(), false, event.ShouldSmartReplace(),
                                     event.ShouldMatchStyle(), action, event.GetMailBlockquoteHandling());
      }
      else
      {
        ReplaceSelectionWithText(event.Data(), false, event.ShouldSmartReplace(), action);
      }
      return true;
    }

    const auto &data = event.Data();
    if (data == u8"\n")
    {
      if (event.IsLineBreak())
      {
        return InsertLineBreak();
      }
      return InsertParagraphSeparator();
    }

    return InsertTextWithoutSendingTextEvent(data, false, &event);
  }

  // FIXME: The kill ring (addRangeToKillRing, setStartNewKillRingSequence), with the kill ring.
  bool Editor::DeleteWithDirection(SelectionDirection direction, TextGranularity granularity,
                                   bool shouldAddToKillRing, bool isTypingAction) noexcept
  {
    if (!CanEdit())
    {
      return false;
    }

    if (_document.Selection().IsRange())
    {
      if (isTypingAction)
      {
        TypingCommand::DeleteKeyPressed(
          _document,
          CanSmartCopyOrDelete() ? EnumSet<TypingCommand::Option> {TypingCommand::Option::SmartDelete}
                                 : EnumSet<TypingCommand::Option> {},
          granularity);
        RevealSelectionAfterEditingOperation();
      }
      else
      {
        DeleteSelectionWithSmartDelete(CanSmartCopyOrDelete());
        // Implicitly calls revealSelectionAfterEditingOperation().
      }
    }
    else
    {
      EnumSet<TypingCommand::Option> options;
      if (CanSmartCopyOrDelete())
      {
        options.Add(TypingCommand::Option::SmartDelete);
      }
      if (shouldAddToKillRing)
      {
        options.Add(TypingCommand::Option::AddsToKillRing);
      }
      switch (direction)
      {
        case SelectionDirection::Forward:
        case SelectionDirection::Right:
          TypingCommand::ForwardDeleteKeyPressed(_document, options, granularity);
          break;
        case SelectionDirection::Backward:
        case SelectionDirection::Left:
          TypingCommand::DeleteKeyPressed(_document, options, granularity);
          break;
      }
      RevealSelectionAfterEditingOperation();
    }

    return true;
  }

  void Editor::DeleteSelectionWithSmartDelete(bool smartDelete, EditAction editingAction) noexcept
  {
    if (_document.Selection().IsNone())
    {
      return;
    }

    DeleteSelectionCommand::Create(_document, smartDelete, true, false, false, true, editingAction)->Apply();
  }

  // FIXME: Beeping when there's nothing to delete (systemBeep), with system sounds; and the kill ring.
  void Editor::PerformDelete() noexcept
  {
    if (!CanDelete())
    {
      return;
    }

    DeleteSelectionWithSmartDelete(CanSmartCopyOrDelete());
  }

  // FIXME: Waiting for the inserted images to load before revealing the selection, with image loading;
  // accessibility's notifications of the pasted text, with accessibility; and checking the spelling of the
  // editable root, with text checking.
  void Editor::ReplaceSelectionWithFragment(dom::DocumentFragment &fragment, bool selectReplacement,
                                            bool smartReplace, bool matchStyle, EditAction editingAction,
                                            MailBlockquoteHandling mailBlockquoteHandling) noexcept
  {
    VisibleSelection selection = _document.Selection().Selection();
    if (selection.IsNone() || !selection.IsContentEditable())
    {
      return;
    }

    EnumSet<ReplaceSelectionCommand::CommandOption> options {
      ReplaceSelectionCommand::CommandOption::PreventNesting,
      ReplaceSelectionCommand::CommandOption::SanitizeFragment};
    if (selectReplacement)
    {
      options.Add(ReplaceSelectionCommand::CommandOption::SelectReplacement);
    }
    if (smartReplace)
    {
      options.Add(ReplaceSelectionCommand::CommandOption::SmartReplace);
    }
    if (matchStyle)
    {
      options.Add(ReplaceSelectionCommand::CommandOption::MatchStyle);
    }
    if (mailBlockquoteHandling == MailBlockquoteHandling::IgnoreBlockquote)
    {
      options.Add(ReplaceSelectionCommand::CommandOption::IgnoreMailBlockquote);
    }

    auto command = ReplaceSelectionCommand::Create(_document, ShareRefPtr(&fragment), options, editingAction);
    command->Apply();

    RevealSelectionAfterEditingOperation();
  }

  void Editor::ReplaceSelectionWithText(utf8_stringview text, bool selectReplacement, bool smartReplace,
                                        EditAction editingAction) noexcept
  {
    auto range = SelectedRange();
    if (!range)
    {
      return;
    }

    ReplaceSelectionWithFragment(*CreateFragmentFromText(*range, text), selectReplacement, smartReplace, true,
                                 editingAction);
  }

  // FIXME: Ignoring selection changes (setIgnoreSelectionChanges), with input methods.
  void Editor::RevealSelectionAfterEditingOperation(const ScrollAlignment &alignment,
                                                    RevealExtentOption revealExtentOption) noexcept
  {
    _document.Selection().RevealSelection({SelectionRevealMode::Reveal, alignment, revealExtentOption});
  }

  void Editor::RevealSelectionAfterEditingOperation() noexcept
  {
    RevealSelectionAfterEditingOperation(ScrollAlignment::AlignCenterIfNeeded,
                                         RevealExtentOption::DoNotRevealExtent);
  }

  // FIXME: A text control's own selection when the event's target is in one, with form controls.
  const VisibleSelection &Editor::SelectionForCommand(dom::Event *) const noexcept
  {
    return _document.Selection().Selection();
  }

  utf8_string Editor::SelectedText() const noexcept
  {
    return SelectedText(TextIteratorBehavior::TraversesFlatTree);
  }

  utf8_string Editor::SelectedTextForDataTransfer() const noexcept
  {
    return SelectedText({TextIteratorBehavior::EmitsImageAltText, TextIteratorBehavior::TraversesFlatTree});
  }

  utf8_string Editor::SelectedText(TextIteratorBehaviors behaviors) const noexcept
  {
    // We remove '\0' characters because they are not visibly rendered to the user.
    auto range = _document.Selection().Selection().FirstRange();
    if (!range)
    {
      return {};
    }
    auto text = PlainText(*range, behaviors);
    std::erase(text, u8'\0');
    return text;
  }
}

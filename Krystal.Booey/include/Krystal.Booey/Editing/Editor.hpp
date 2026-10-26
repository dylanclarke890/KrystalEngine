#pragma once

#include "Krystal.Booey/Editing/CompositionUnderline.hpp"
#include "Krystal.Booey/Editing/EditAction.hpp"
#include "Krystal.Booey/Editing/EditingBehavior.hpp"
#include "Krystal.Booey/Editing/TextIteratorBehavior.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::dom
{
  class DataTransfer;
  class Document;
  class DocumentFragment;
  class Element;
  class Event;
  class KeyboardEvent;
  class StaticRange;
  class Text;
  class TextEvent;
  enum class TextEventInputType : uint8;
  struct SimpleRange;
}

namespace krys::boo
{
  class CompositeEditCommand;
  class EditCommandComposition;
  class EditorClient;
  class EditorInternalCommand;
  class IntRect;
  class LocalFrame;
  class Pasteboard;
  class VisibleSelection;
  struct ScrollAlignment;
  enum class FrameSelectionSetSelectionOption : uint8;
  enum class RevealExtentOption : bool;
  enum class SelectionDirection : uint8;
  enum class TextGranularity : uint8;

  enum class EditorCommandSource : uint8
  {
    MenuOrKeyBinding,
    DOM,
    DOMWithUserInterface
  };

  enum class EditorParagraphSeparator : bool
  {
    div,
    p
  };

  enum class MailBlockquoteHandling : bool
  {
    RespectBlockquote,
    IgnoreBlockquote,
  };

  enum class EditorPasteOption : uint8
  {
    AllowPlainText,
    IgnoreMailBlockquote,
    AsQuotation,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::EditorPasteOption, 3uz);

namespace krys::boo
{
  enum class EditorActionSpecifier : bool
  {
    CutAction,
    CopyAction
  };

  enum class ClipboardEventKind : uint8
  {
    Copy,
    CopyFont,
    Cut,
    Paste,
    PasteAsPlainText,
    PasteAsQuotation,
    PasteFont,
    BeforeCopy,
    BeforeCut,
    BeforePaste
  };

  /// @brief A document's editor: the commands that act on its selection (moving and extending it, selecting
  /// and copying), the keys bound to them, the clipboard, and the text an input method composes.
  // FIXME: Editing commands (typing, deleting, cutting, pasting, styling, undo and redo), the typing style,
  // spelling and grammar, text checking, the mark and kill ring, find, and the editor's responses to
  // selection and content changes, with editing commands, form controls and text input.
  class Editor
  {
    KRYS_NON_COPY_MOVABLE(Editor);

  private:
    dom::Document &_document;
    RefPtr<CompositeEditCommand> _lastEditCommand;
    EditorParagraphSeparator _defaultParagraphSeparator {EditorParagraphSeparator::div};
    bool _copyingFromMenuOrKeyBinding {false};
    bool _pastingFromMenuOrKeyBinding {false};
    bool _overwriteModeEnabled {false};
    bool _ignoreSelectionChanges {false};

    RefPtr<dom::Text> _compositionNode;
    uint32 _compositionStart {0};
    uint32 _compositionEnd {0};
    List<CompositionUnderline> _customCompositionUnderlines;

  public:
    enum class FromMenuOrKeyBinding : bool
    {
      No,
      Yes
    };

    enum class RevealSelection : bool
    {
      No,
      Yes
    };

    /// @brief An editor command by name, bound to the document and the source it came from.
    class Command
    {
    private:
      const EditorInternalCommand *_command {nullptr};
      EditorCommandSource _source {EditorCommandSource::MenuOrKeyBinding};
      RefPtr<dom::Document> _document;

    public:
      Command() noexcept = default;
      Command(const EditorInternalCommand *command, EditorCommandSource source,
              dom::Document &document) noexcept;

      bool Execute(utf8_stringview parameter = {}, dom::Event *triggeringEvent = nullptr) const noexcept;
      bool Execute(dom::Event *triggeringEvent) const noexcept;

      KRYS_NODISCARD bool IsSupported() const noexcept;
      KRYS_NODISCARD bool IsEnabled(dom::Event *triggeringEvent = nullptr) const noexcept;

      KRYS_NODISCARD bool IsTextInsertion() const noexcept;
      KRYS_NODISCARD bool AllowExecutionWhenDisabled() const noexcept;

    private:
      KRYS_NODISCARD LocalFrame *Frame() const noexcept;
    };

    explicit Editor(dom::Document &document) noexcept;
    ~Editor() noexcept;

    KRYS_NODISCARD dom::Document &GetDocument() const noexcept
    {
      return _document;
    }

    /// @brief The host's side of the editor; null when the document has no page.
    KRYS_NODISCARD EditorClient *Client() const noexcept;

    /// @brief Lets the editor client run the command @p event is bound to.
    /// @return Whether it did, so the event's default action is done.
    bool HandleKeyboardEvent(dom::KeyboardEvent &event) noexcept;
    /// @brief Lets the editor client mark a `keydown` an input method handled, before it's dispatched.
    void HandleInputMethodKeydown(dom::KeyboardEvent &event) noexcept;
    /// @brief Lets the editor client apply what an input method made of a key, after its `keydown`.
    void DidDispatchInputMethodKeydown(dom::KeyboardEvent &event) noexcept;

    using PasteOption = EditorPasteOption;

    bool CanDHTMLCut() noexcept;
    bool CanDHTMLCopy() noexcept;
    bool CanDHTMLPaste() noexcept;
    bool TryDHTMLCopy() noexcept;
    bool TryDHTMLCut() noexcept;
    KRYS_NODISCARD bool CanCut() const noexcept;
    KRYS_NODISCARD bool CanCopy() const noexcept;
    KRYS_NODISCARD bool CanPaste() const noexcept;
    void Cut(FromMenuOrKeyBinding fromMenuOrKeyBinding = FromMenuOrKeyBinding::No) noexcept;
    void Copy(FromMenuOrKeyBinding fromMenuOrKeyBinding = FromMenuOrKeyBinding::No) noexcept;
    void Paste(FromMenuOrKeyBinding fromMenuOrKeyBinding = FromMenuOrKeyBinding::No) noexcept;
    void Paste(Pasteboard &pasteboard,
               FromMenuOrKeyBinding fromMenuOrKeyBinding = FromMenuOrKeyBinding::No) noexcept;
    void PasteAsPlainText(FromMenuOrKeyBinding fromMenuOrKeyBinding = FromMenuOrKeyBinding::No) noexcept;
    /// @brief Fires `textInput` for pasting @p pastingText at the selection's target; its default action
    /// replaces the selection.
    void PasteAsPlainText(utf8_stringview pastingText, bool smartReplace) noexcept;
    void PasteAsFragment(
      Ref<dom::DocumentFragment> &&pastingFragment, bool smartReplace, bool matchStyle,
      MailBlockquoteHandling respectsMailBlockquote = MailBlockquoteHandling::RespectBlockquote,
      EditAction action = EditAction::Paste) noexcept;
    KRYS_NODISCARD bool IsPastingFromMenuOrKeyBinding() const noexcept
    {
      return _pastingFromMenuOrKeyBinding;
    }
    KRYS_NODISCARD bool IsCopyingFromMenuOrKeyBinding() const noexcept
    {
      return _copyingFromMenuOrKeyBinding;
    }

    KRYS_NODISCARD utf8_string ReadPlainTextFromPasteboard(Pasteboard &pasteboard) noexcept;
    /// @brief Writes the selection's text and markup (with its styles inlined) to @p pasteboard.
    void WriteSelectionToPasteboard(Pasteboard &pasteboard) noexcept;
    /// @brief The pasteboard's richest content as a fragment to paste at @p context.
    /// @param allowPlainText Whether plain text may be read when there's no richer content.
    /// @param chosePlainText Set to whether the fragment was made from plain text.
    KRYS_NODISCARD RefPtr<dom::DocumentFragment> WebContentFromPasteboard(Pasteboard &pasteboard,
                                                                          const dom::SimpleRange &context,
                                                                          bool allowPlainText,
                                                                          bool &chosePlainText) noexcept;

    /// @brief The command named @p commandName (ignoring ASCII case), from a menu or key binding.
    KRYS_NODISCARD Command GetCommand(utf8_stringview commandName) noexcept;
    KRYS_NODISCARD Command GetCommand(utf8_stringview commandName, EditorCommandSource source) noexcept;
    /// @brief Works without a frame.
    KRYS_NODISCARD static bool CommandIsSupportedFromMenuOrKeyBinding(utf8_stringview commandName) noexcept;

    KRYS_NODISCARD EditingBehavior Behavior() const noexcept;

    void TextFieldDidBeginEditing(dom::Element &element) noexcept;
    void TextFieldDidEndEditing(dom::Element &element) noexcept;
    void TextDidChangeInTextField(dom::Element &element) noexcept;
    bool DoTextFieldCommandFromEvent(dom::Element &element, dom::KeyboardEvent *event) noexcept;
    void TextWillBeDeletedInTextField(dom::Element &input) noexcept;
    void TextDidChangeInTextArea(dom::Element &element) noexcept;

    /// @brief The element a new paragraph is made of: a `div`, or a `p`.
    KRYS_NODISCARD EditorParagraphSeparator DefaultParagraphSeparator() const noexcept
    {
      return _defaultParagraphSeparator;
    }
    void SetDefaultParagraphSeparator(EditorParagraphSeparator separator) noexcept
    {
      _defaultParagraphSeparator = separator;
    }

    /// @brief The selection a command triggered by @p event works on: a text control's own selection when
    /// the event is at one the frame's selection isn't in.
    KRYS_NODISCARD VisibleSelection SelectionForCommand(dom::Event *event) const noexcept;

    KRYS_NODISCARD utf8_string SelectedText() const noexcept;
    KRYS_NODISCARD utf8_string SelectedTextForDataTransfer() const noexcept;

    KRYS_NODISCARD bool CanEdit() const noexcept;
    KRYS_NODISCARD bool CanEditRichly() const noexcept;
    KRYS_NODISCARD bool CanDelete() const noexcept;
    KRYS_NODISCARD bool CanDeleteRange(const dom::SimpleRange &range) const noexcept;
    KRYS_NODISCARD bool ShouldDeleteRange(const Maybe<dom::SimpleRange> &range) const noexcept;
    KRYS_NODISCARD bool SmartInsertDeleteEnabled() noexcept;
    KRYS_NODISCARD bool CanSmartCopyOrDelete() noexcept;
    KRYS_NODISCARD Maybe<dom::SimpleRange> SelectedRange() noexcept;

    /// @brief Inserts typed text, firing `textInput` at the event's target first.
    bool InsertText(utf8_stringview text, dom::Event *triggeringEvent,
                    dom::TextEventInputType inputType) noexcept;
    bool InsertText(utf8_stringview text, dom::Event *triggeringEvent) noexcept;
    bool InsertTextWithoutSendingTextEvent(utf8_stringview text, bool selectInsertedText,
                                           dom::TextEvent *triggeringEvent) noexcept;
    bool InsertLineBreak() noexcept;
    bool InsertParagraphSeparator() noexcept;
    bool InsertParagraphSeparatorInQuotedContent() noexcept;
    /// @brief The default action of a `textInput` event: inserts its text, or pastes its fragment.
    /// @return Whether it was handled.
    bool HandleTextEvent(dom::TextEvent &event) noexcept;

    bool DeleteWithDirection(SelectionDirection direction, TextGranularity granularity,
                             bool shouldAddToKillRing, bool isTypingAction) noexcept;
    void DeleteSelectionWithSmartDelete(bool smartDelete,
                                        EditAction editingAction = EditAction::Delete) noexcept;
    void PerformDelete() noexcept;

    void ReplaceSelectionWithFragment(
      dom::DocumentFragment &fragment, bool selectReplacement, bool smartReplace, bool matchStyle,
      EditAction editingAction = EditAction::Insert,
      MailBlockquoteHandling mailBlockquoteHandling = MailBlockquoteHandling::RespectBlockquote) noexcept;
    void ReplaceSelectionWithText(utf8_stringview text, bool selectReplacement, bool smartReplace,
                                  EditAction editingAction = EditAction::Insert) noexcept;

    void RevealSelectionAfterEditingOperation(const ScrollAlignment &alignment,
                                              RevealExtentOption revealExtentOption) noexcept;
    void RevealSelectionAfterEditingOperation() noexcept;

    /// @brief Fires `beforeinput` for a command about to be applied.
    /// @return Whether to go on and apply it.
    bool WillApplyEditing(CompositeEditCommand &command,
                          List<RefPtr<dom::StaticRange>> &&targetRanges) noexcept;
    /// @brief Selects the command's result, fires `input`, and registers the command to be undone.
    void AppliedEditing(CompositeEditCommand &command) noexcept;
    bool WillUnapplyEditing(const EditCommandComposition &composition) const noexcept;
    void UnappliedEditing(EditCommandComposition &composition) noexcept;
    bool WillReapplyEditing(const EditCommandComposition &composition) const noexcept;
    void ReappliedEditing(EditCommandComposition &composition) noexcept;

    KRYS_NODISCARD bool CanUndo() const noexcept;
    void Undo() noexcept;
    KRYS_NODISCARD bool CanRedo() const noexcept;
    void Redo() noexcept;

    KRYS_NODISCARD CompositeEditCommand *LastEditCommand() const noexcept;

    /// @brief Whether typing replaces the characters after the caret rather than inserting before them.
    KRYS_NODISCARD bool IsOverwriteModeEnabled() const noexcept
    {
      return _overwriteModeEnabled;
    }
    // FIXME: Showing a block cursor while overwriting (setShouldShowBlockCursor), with carets.
    void ToggleOverwriteModeEnabled() noexcept
    {
      _overwriteModeEnabled = !_overwriteModeEnabled;
    }

    void RespondToChangedContents(const VisibleSelection &endingSelection) noexcept;
    /// @brief Tells the editor client the selection changed.
    void RespondToChangedSelection(const VisibleSelection &oldSelection,
                                   EnumSet<FrameSelectionSetSelectionOption> options) noexcept;

    /// @brief Inserts the text an input method confirmed, firing `textInput` first.
    bool InsertTextForConfirmedComposition(utf8_stringview text) noexcept;

    // International text input composition.
    KRYS_NODISCARD bool HasComposition() const noexcept;
    /// @brief Sets the text an input method composes, replacing the composition (or else the selection) with
    /// @p text, underlined by @p underlines, with @p selectionStart to @p selectionEnd selected (offsets in
    /// @p text's code units); empty @p text cancels the composition. Fires `compositionstart` (with the text
    /// it replaces) when one starts, `compositionupdate` while it has text, and `compositionend` when it's
    /// cancelled.
    // FIXME: The host's custom highlights and annotations (Cocoa's inline predictions).
    void SetComposition(utf8_stringview text, const List<CompositionUnderline> &underlines,
                        uint32 selectionStart, uint32 selectionEnd) noexcept;
    /// @brief Confirms the composition as it is: its text stays, and `compositionend` fires.
    void ConfirmComposition() noexcept;
    /// @brief Confirms the composition as @p text, or inserts @p text in place of the selection if there's
    /// none.
    void ConfirmComposition(utf8_stringview text) noexcept;
    /// @brief Confirms the composition, or cancels it if the selection has left it, and tells the client.
    void ConfirmOrCancelCompositionAndNotifyClient() noexcept;
    /// @brief Drops the composition, leaving its text as it is, and `compositionend` fires with no data (to
    /// remove the text, set an empty composition).
    void CancelComposition() noexcept;
    /// @brief Cancels the composition if the selection isn't within it.
    /// @return Whether it did.
    bool CancelCompositionIfSelectionIsInvalid() noexcept;
    /// @brief The composition's text, or null if there's none.
    KRYS_NODISCARD Maybe<dom::SimpleRange> CompositionRange() const noexcept;
    /// @brief The selection within the composition, relative to its start.
    /// @return Whether the selection is within the composition.
    bool GetCompositionSelection(uint32 &selectionStart, uint32 &selectionEnd) const noexcept;

    /// @brief The text node the composition is in, for painting it.
    KRYS_NODISCARD dom::Text *CompositionNode() const noexcept;
    KRYS_NODISCARD uint32 CompositionStart() const noexcept
    {
      return _compositionStart;
    }
    KRYS_NODISCARD uint32 CompositionEnd() const noexcept
    {
      return _compositionEnd;
    }
    KRYS_NODISCARD bool CompositionUsesCustomUnderlines() const noexcept
    {
      return !_customCompositionUnderlines.empty();
    }
    /// @brief The composition's underlines, with offsets in its text node.
    KRYS_NODISCARD const List<CompositionUnderline> &CustomCompositionUnderlines() const noexcept
    {
      return _customCompositionUnderlines;
    }

    /// @brief Ignores selection changes (they don't cancel the composition) until called with false.
    void SetIgnoreSelectionChanges(
      bool ignore, RevealSelection shouldRevealExistingSelection = RevealSelection::Yes) noexcept;
    KRYS_NODISCARD bool IgnoreSelectionChanges() const noexcept
    {
      return _ignoreSelectionChanges;
    }

    /// @brief The first line's rect of @p range in absolute coordinates (the caret's, without its width, when
    /// it's collapsed), for placing an input method's windows.
    KRYS_NODISCARD IntRect FirstRectForRange(const dom::SimpleRange &range) const noexcept;

    void CloseTyping() noexcept;

  private:
    enum class SetCompositionMode : bool
    {
      ConfirmComposition,
      CancelComposition
    };

    void SelectComposition() noexcept;
    void SetComposition(utf8_stringview text, SetCompositionMode mode) noexcept;

    KRYS_NODISCARD utf8_string SelectedText(TextIteratorBehaviors behaviors) const noexcept;

    bool DispatchClipboardEvent(RefPtr<dom::Element> &&target, ClipboardEventKind kind) noexcept;
    bool DispatchClipboardEvent(RefPtr<dom::Element> &&target, ClipboardEventKind kind,
                                Ref<dom::DataTransfer> &&dataTransfer) noexcept;
    KRYS_NODISCARD RefPtr<dom::Element> FindEventTargetFrom(const VisibleSelection &selection) const noexcept;
    KRYS_NODISCARD RefPtr<dom::Element> FindEventTargetFromSelection() const noexcept;

    void PerformCutOrCopy(EditorActionSpecifier action) noexcept;

    void PasteAsPlainTextWithPasteboard(Pasteboard &pasteboard) noexcept;
    void PasteWithPasteboard(Pasteboard *pasteboard, EnumSet<PasteOption> options) noexcept;
    KRYS_NODISCARD bool CanSmartReplaceWithPasteboard(Pasteboard &pasteboard) noexcept;
    KRYS_NODISCARD bool ShouldInsertFragment(dom::DocumentFragment &fragment,
                                             const Maybe<dom::SimpleRange> &replacingDOMRange) const noexcept;

    void ChangeSelectionAfterCommand(const VisibleSelection &newSelection,
                                     EnumSet<FrameSelectionSetSelectionOption> options) noexcept;

    KRYS_NODISCARD bool ShouldSmartDelete() noexcept;
    KRYS_NODISCARD bool ShouldInsertText(utf8_stringview text,
                                         const Maybe<dom::SimpleRange> &range) const noexcept;
  };
}

#pragma once

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
  class Document;
  class DocumentFragment;
  class Element;
  class Event;
  class KeyboardEvent;
  class StaticRange;
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
  /// and copying), the keys bound to them, and the clipboard.
  // FIXME: Editing commands (typing, deleting, cutting, pasting, styling, undo and redo), the typing style,
  // spelling and grammar, text checking, input methods, the mark and kill ring, find, and the editor's
  // responses to selection and content changes, with editing commands, form controls and text input.
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

  public:
    enum class FromMenuOrKeyBinding : bool
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

  private:
    KRYS_NODISCARD utf8_string SelectedText(TextIteratorBehaviors behaviors) const noexcept;

    bool DispatchClipboardEvent(RefPtr<dom::Element> &&target, ClipboardEventKind kind) noexcept;
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

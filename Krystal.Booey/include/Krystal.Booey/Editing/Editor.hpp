#pragma once

#include "Krystal.Booey/Editing/EditingBehavior.hpp"
#include "Krystal.Booey/Editing/TextIteratorBehavior.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;
  class Event;
  class KeyboardEvent;
}

namespace krys::boo
{
  class EditorClient;
  class EditorInternalCommand;
  class LocalFrame;
  class VisibleSelection;

  enum class EditorCommandSource : uint8
  {
    MenuOrKeyBinding,
    DOM,
    DOMWithUserInterface
  };

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
    bool _copyingFromMenuOrKeyBinding {false};

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

    bool CanDHTMLCopy() noexcept;
    bool TryDHTMLCopy() noexcept;
    KRYS_NODISCARD bool CanCopy() const noexcept;
    void Copy(FromMenuOrKeyBinding fromMenuOrKeyBinding = FromMenuOrKeyBinding::No) noexcept;

    /// @brief The command named @p commandName (ignoring ASCII case), from a menu or key binding.
    KRYS_NODISCARD Command GetCommand(utf8_stringview commandName) noexcept;
    KRYS_NODISCARD Command GetCommand(utf8_stringview commandName, EditorCommandSource source) noexcept;
    /// @brief Works without a frame.
    KRYS_NODISCARD static bool CommandIsSupportedFromMenuOrKeyBinding(utf8_stringview commandName) noexcept;

    KRYS_NODISCARD EditingBehavior Behavior() const noexcept;

    KRYS_NODISCARD const VisibleSelection &SelectionForCommand(dom::Event *event) const noexcept;

    KRYS_NODISCARD utf8_string SelectedText() const noexcept;
    KRYS_NODISCARD utf8_string SelectedTextForDataTransfer() const noexcept;

  private:
    KRYS_NODISCARD utf8_string SelectedText(TextIteratorBehaviors behaviors) const noexcept;

    bool DispatchClipboardEvent(RefPtr<dom::Element> &&target, ClipboardEventKind kind) noexcept;
    KRYS_NODISCARD RefPtr<dom::Element> FindEventTargetFrom(const VisibleSelection &selection) const noexcept;
    KRYS_NODISCARD RefPtr<dom::Element> FindEventTargetFromSelection() const noexcept;

    void PerformCutOrCopy(EditorActionSpecifier action) noexcept;
  };
}

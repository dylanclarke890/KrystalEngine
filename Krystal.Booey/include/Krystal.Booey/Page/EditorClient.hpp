#pragma once

#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/Editing/TextAffinity.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;
  class KeyboardEvent;
}

namespace krys::boo
{
  class LocalFrame;
  class UndoStep;

  /// @brief What a page's editor asks of its host: the commands keys run, input methods, and whether
  /// selections and clipboard access are allowed.
  // FIXME: The rest of the editor client: editing delegates (shouldBeginEditing, shouldInsertText,
  // shouldApplyStyle and their notifications), spelling and grammar, and text checking.
  class EditorClient
  {
    KRYS_NON_COPY_MOVABLE(EditorClient);

  public:
    EditorClient() noexcept = default;
    virtual ~EditorClient() noexcept = default;

    KRYS_NODISCARD virtual bool ShouldChangeSelectedRange(const Maybe<dom::SimpleRange> &fromRange,
                                                          const Maybe<dom::SimpleRange> &toRange,
                                                          Affinity affinity,
                                                          bool stillSelecting) noexcept = 0;

    KRYS_NODISCARD virtual bool CanCopyCut(LocalFrame *frame, bool defaultValue) const noexcept = 0;
    KRYS_NODISCARD virtual bool CanPaste(LocalFrame *frame, bool defaultValue) const noexcept = 0;

    /// @brief Runs the editor command a `keydown` or `keypress` is bound to, if any.
    /// @return Whether the event was handled, so its default action is done.
    virtual bool HandleKeyboardEvent(dom::KeyboardEvent &event) noexcept = 0;

    virtual void RespondToChangedContents() noexcept = 0;
    /// @brief Called after the selection changed (unless the change asked not to tell the editor's clients).
    virtual void RespondToChangedSelection(LocalFrame *frame) noexcept = 0;
    /// @brief Called when the page dropped the input method's composition (confirming or cancelling it), so
    /// the host can tell the input method.
    virtual void DiscardedComposition(const dom::Document &document) noexcept = 0;
    /// @brief Called after the composition was set, confirmed or cancelled.
    virtual void DidUpdateComposition() noexcept = 0;

    /// @brief Called before a `keydown` is dispatched: marks it handled (its default handled) if an input
    /// method handled the key, so it goes out with key code 229 and no `keypress`.
    virtual void HandleInputMethodKeydown(dom::KeyboardEvent &event) noexcept = 0;
    /// @brief Called after a `keydown` an input method handled was dispatched, to apply what it made of the
    /// key: the composition, or the text it confirmed.
    virtual void DidDispatchInputMethodKeydown(dom::KeyboardEvent &) noexcept
    {
    }

    /// @brief Called as the focused element changes (@p element; null when nothing is), so the host can turn
    /// its input method on for elements that use one (`Element::ShouldUseInputMethod()`) and off otherwise.
    virtual void SetInputMethodState(dom::Element *element) noexcept = 0;

    /// @brief Whether deleting and pasting whole words adds and removes the spaces around them.
    KRYS_NODISCARD virtual bool SmartInsertDeleteEnabled() noexcept = 0;

    /// @brief Keeps an edit (or an undone edit that was redone) so it can be undone.
    virtual void RegisterUndoStep(UndoStep &step) noexcept = 0;
    /// @brief Keeps an undone edit so it can be redone.
    virtual void RegisterRedoStep(UndoStep &step) noexcept = 0;
    virtual void ClearUndoRedoOperations() noexcept = 0;

    KRYS_NODISCARD virtual bool CanUndo() const noexcept = 0;
    KRYS_NODISCARD virtual bool CanRedo() const noexcept = 0;
    virtual void Undo() noexcept = 0;
    virtual void Redo() noexcept = 0;

    /// @brief Called when a text field is focused, so the host can offer to fill it (autofill).
    virtual void TextFieldDidBeginEditing(dom::Element &element) noexcept = 0;
    /// @brief Called when a text field loses the focus.
    virtual void TextFieldDidEndEditing(dom::Element &element) noexcept = 0;
    /// @brief Called after the user changed the text of the focused text field.
    virtual void TextDidChangeInTextField(dom::Element &element) noexcept = 0;
    /// @brief Lets the host handle a key in a focused text field (to move through its suggestions).
    /// @return Whether the host handled it.
    virtual bool DoTextFieldCommandFromEvent(dom::Element &element, dom::KeyboardEvent *event) noexcept = 0;
    virtual void TextWillBeDeletedInTextField(dom::Element &element) noexcept = 0;
    /// @brief Called after the user changed the text of the focused text area.
    virtual void TextDidChangeInTextArea(dom::Element &element) noexcept = 0;
  };

  /// @brief The editor client of a page that has no host.
  class EmptyEditorClient final : public EditorClient
  {
  public:
    KRYS_NODISCARD bool ShouldChangeSelectedRange(const Maybe<dom::SimpleRange> &,
                                                  const Maybe<dom::SimpleRange> &, Affinity,
                                                  bool) noexcept final
    {
      return false;
    }

    KRYS_NODISCARD bool CanCopyCut(LocalFrame *, bool defaultValue) const noexcept final
    {
      return defaultValue;
    }

    KRYS_NODISCARD bool CanPaste(LocalFrame *, bool defaultValue) const noexcept final
    {
      return defaultValue;
    }

    bool HandleKeyboardEvent(dom::KeyboardEvent &) noexcept final
    {
      return false;
    }

    void RespondToChangedContents() noexcept final
    {
    }

    void RespondToChangedSelection(LocalFrame *) noexcept final
    {
    }

    void DiscardedComposition(const dom::Document &) noexcept final
    {
    }

    void DidUpdateComposition() noexcept final
    {
    }

    void HandleInputMethodKeydown(dom::KeyboardEvent &) noexcept final
    {
    }

    void SetInputMethodState(dom::Element *) noexcept final
    {
    }

    KRYS_NODISCARD bool SmartInsertDeleteEnabled() noexcept final
    {
      return false;
    }

    void RegisterUndoStep(UndoStep &) noexcept final
    {
    }
    void RegisterRedoStep(UndoStep &) noexcept final
    {
    }
    void ClearUndoRedoOperations() noexcept final
    {
    }

    KRYS_NODISCARD bool CanUndo() const noexcept final
    {
      return false;
    }
    KRYS_NODISCARD bool CanRedo() const noexcept final
    {
      return false;
    }
    void Undo() noexcept final
    {
    }
    void Redo() noexcept final
    {
    }

    void TextFieldDidBeginEditing(dom::Element &) noexcept final
    {
    }

    void TextFieldDidEndEditing(dom::Element &) noexcept final
    {
    }

    void TextDidChangeInTextField(dom::Element &) noexcept final
    {
    }

    bool DoTextFieldCommandFromEvent(dom::Element &, dom::KeyboardEvent *) noexcept final
    {
      return false;
    }

    void TextWillBeDeletedInTextField(dom::Element &) noexcept final
    {
    }

    void TextDidChangeInTextArea(dom::Element &) noexcept final
    {
    }
  };
}

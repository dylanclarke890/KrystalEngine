#pragma once

#include "Krystal.Booey/Editing/UndoStep.hpp"
#include "Krystal.Booey/Page/EditorClient.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::ui
{
  /// @brief A view's editor client: runs the editor commands Windows binds keys to (Shift with the arrows to
  /// extend the selection, Ctrl+A to select all, Ctrl+C to copy, and the rest), allows selection changes and
  /// copying, keeps the undo and redo stacks, and applies the input method's composition, which comes as key
  /// presses it handled.
  class EditorClient final : public boo::EditorClient
  {
  private:
    List<Ref<boo::UndoStep>> _undoStack;
    List<Ref<boo::UndoStep>> _redoStack;
    bool _isInRedo {false};
    /// @brief Whether the focused element takes text from the input method.
    bool _acceptsTextInput {false};
    /// @brief Tells the host the page dropped the composition, so its input method drops its own.
    Func<void()> _discardedCompositionHandler;

  public:
    KRYS_NODISCARD bool ShouldChangeSelectedRange(const Maybe<boo::dom::SimpleRange> &fromRange,
                                                  const Maybe<boo::dom::SimpleRange> &toRange,
                                                  boo::Affinity affinity,
                                                  bool stillSelecting) noexcept override;

    KRYS_NODISCARD bool CanCopyCut(boo::LocalFrame *frame, bool defaultValue) const noexcept override;
    KRYS_NODISCARD bool CanPaste(boo::LocalFrame *frame, bool defaultValue) const noexcept override;

    bool HandleKeyboardEvent(boo::dom::KeyboardEvent &event) noexcept override;

    void RespondToChangedContents() noexcept override
    {
    }

    void RespondToChangedSelection(boo::LocalFrame *frame) noexcept override;
    void DiscardedComposition(const boo::dom::Document &document) noexcept override;

    void DidUpdateComposition() noexcept override
    {
    }

    void HandleInputMethodKeydown(boo::dom::KeyboardEvent &event) noexcept override;
    void DidDispatchInputMethodKeydown(boo::dom::KeyboardEvent &event) noexcept override;
    void SetInputMethodState(boo::dom::Element *element) noexcept override;

    /// @brief Whether the focused element takes text from the input method.
    KRYS_NODISCARD bool AcceptsTextInput() const noexcept
    {
      return _acceptsTextInput;
    }

    /// @brief Calls @p handler when the page drops the composition (cancelling it as the selection left it),
    /// so the host's input method drops its own.
    void SetDiscardedCompositionHandler(Func<void()> &&handler) noexcept
    {
      _discardedCompositionHandler = krys::move(handler);
    }

    KRYS_NODISCARD bool SmartInsertDeleteEnabled() noexcept override
    {
      return false;
    }

    void RegisterUndoStep(boo::UndoStep &step) noexcept override;
    void RegisterRedoStep(boo::UndoStep &step) noexcept override;
    void ClearUndoRedoOperations() noexcept override;

    KRYS_NODISCARD bool CanUndo() const noexcept override;
    KRYS_NODISCARD bool CanRedo() const noexcept override;
    void Undo() noexcept override;
    void Redo() noexcept override;

    // FIXME: Autofill (offering to fill text fields as they're edited), which the host doesn't do.

    void TextFieldDidBeginEditing(boo::dom::Element &) noexcept override
    {
    }

    void TextFieldDidEndEditing(boo::dom::Element &) noexcept override
    {
    }

    void TextDidChangeInTextField(boo::dom::Element &) noexcept override
    {
    }

    bool DoTextFieldCommandFromEvent(boo::dom::Element &, boo::dom::KeyboardEvent *) noexcept override
    {
      return false;
    }

    void TextWillBeDeletedInTextField(boo::dom::Element &) noexcept override
    {
    }

    void TextDidChangeInTextArea(boo::dom::Element &) noexcept override
    {
    }

    /// @brief The name of the editor command @p event is bound to, or empty if none is.
    KRYS_NODISCARD static utf8_stringview InterpretKeyEvent(const boo::dom::KeyboardEvent &event) noexcept;
  };
}

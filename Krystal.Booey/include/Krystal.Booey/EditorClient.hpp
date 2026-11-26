#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Web/Editing/Commands/UndoStep.hpp"
#include "Krystal.Web/Page/Clients/EditorClient.hpp"

namespace krys::boo
{
  /// @brief A view's editor client: runs the editor commands Windows binds keys to (Shift with the arrows to
  /// extend the selection, Ctrl+A to select all, Ctrl+C to copy, and the rest), allows selection changes and
  /// copying, keeps the undo and redo stacks, and applies the input method's composition, which comes as key
  /// presses it handled.
  class EditorClient final : public web::EditorClient
  {
  private:
    List<Ref<web::UndoStep>> _undoStack;
    List<Ref<web::UndoStep>> _redoStack;
    bool _isInRedo {false};
    /// @brief Whether the focused element takes text from the input method.
    bool _acceptsTextInput {false};
    /// @brief Tells the host the page dropped the composition, so its input method drops its own.
    Func<void()> _discardedCompositionHandler;

  public:
    KRYS_NODISCARD bool ShouldChangeSelectedRange(const Maybe<web::dom::SimpleRange> &fromRange,
                                                  const Maybe<web::dom::SimpleRange> &toRange,
                                                  web::Affinity affinity,
                                                  bool stillSelecting) noexcept override;

    KRYS_NODISCARD bool CanCopyCut(web::Frame *frame, bool defaultValue) const noexcept override;

    KRYS_NODISCARD bool CanPaste(web::Frame *frame, bool defaultValue) const noexcept override;

    bool HandleKeyboardEvent(web::KeyboardEvent &event) noexcept override;

    void RespondToChangedContents() noexcept override
    {
    }

    void RespondToChangedSelection(web::Frame *frame) noexcept override;

    void DiscardedComposition(const web::Document &document) noexcept override;

    void DidUpdateComposition() noexcept override
    {
    }

    void HandleInputMethodKeydown(web::KeyboardEvent &event) noexcept override;

    void DidDispatchInputMethodKeydown(web::KeyboardEvent &event) noexcept override;

    void SetInputMethodState(web::Element *element) noexcept override;

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

    void RegisterUndoStep(web::UndoStep &step) noexcept override;

    void RegisterRedoStep(web::UndoStep &step) noexcept override;

    void ClearUndoRedoOperations() noexcept override;

    KRYS_NODISCARD bool CanUndo() const noexcept override;

    KRYS_NODISCARD bool CanRedo() const noexcept override;

    void Undo() noexcept override;

    void Redo() noexcept override;

    // There's no autofill (not planned, see Krystal.Web's OVERVIEW.md).

    void TextFieldDidBeginEditing(web::Element &) noexcept override
    {
    }

    void TextFieldDidEndEditing(web::Element &) noexcept override
    {
    }

    void TextDidChangeInTextField(web::Element &) noexcept override
    {
    }

    bool DoTextFieldCommandFromEvent(web::Element &, web::KeyboardEvent *) noexcept override
    {
      return false;
    }

    void TextWillBeDeletedInTextField(web::Element &) noexcept override
    {
    }

    void TextDidChangeInTextArea(web::Element &) noexcept override
    {
    }

    /// @brief The name of the editor command @p event is bound to, or empty if none is.
    KRYS_NODISCARD static utf8_stringview InterpretKeyEvent(const web::KeyboardEvent &event) noexcept;
  };
}

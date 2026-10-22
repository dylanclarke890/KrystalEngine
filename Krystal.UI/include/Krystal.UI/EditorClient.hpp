#pragma once

#include "Krystal.Booey/Editing/UndoStep.hpp"
#include "Krystal.Booey/Page/EditorClient.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::ui
{
  /// @brief A view's editor client: runs the editor commands Windows binds keys to (Shift with the arrows to
  /// extend the selection, Ctrl+A to select all, Ctrl+C to copy, and the rest), allows selection changes and
  /// copying, and keeps the undo and redo stacks.
  class EditorClient final : public boo::EditorClient
  {
  private:
    List<Ref<boo::UndoStep>> _undoStack;
    List<Ref<boo::UndoStep>> _redoStack;
    bool _isInRedo {false};

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

    /// @brief The name of the editor command @p event is bound to, or empty if none is.
    KRYS_NODISCARD static utf8_stringview InterpretKeyEvent(const boo::dom::KeyboardEvent &event) noexcept;
  };
}

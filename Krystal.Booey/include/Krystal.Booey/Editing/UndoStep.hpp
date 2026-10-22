#pragma once

#include "Krystal.Booey/Editing/EditAction.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief A step the editor client keeps on its undo and redo stacks.
  class UndoStep : public RefCounted<UndoStep>
  {
  public:
    virtual ~UndoStep() noexcept = default;

    virtual void Unapply() noexcept = 0;
    virtual void Reapply() noexcept = 0;
    KRYS_NODISCARD virtual EditAction EditingAction() const noexcept = 0;
    KRYS_NODISCARD virtual utf8_string Label() const noexcept = 0;
    virtual void DidRemoveFromUndoManager() noexcept = 0;
  };
}

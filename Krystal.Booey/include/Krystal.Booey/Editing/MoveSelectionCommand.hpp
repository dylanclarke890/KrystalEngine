#pragma once

#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/Editing/CompositeEditCommand.hpp"

namespace krys::boo::dom
{
  class DocumentFragment;
}

namespace krys::boo
{
  /// @brief Moves the selection, dropped as @p fragment, to a position: deletes the selection, then inserts
  /// the fragment there.
  class MoveSelectionCommand final : public CompositeEditCommand
  {
  private:
    const Ref<dom::DocumentFragment> _fragment;
    dom::Position _position;
    bool _smartInsert;
    bool _smartDelete;

  public:
    MoveSelectionCommand(Ref<dom::DocumentFragment> &&fragment, const dom::Position &position,
                         bool smartInsert, bool smartDelete) noexcept;

    KRYS_NODISCARD static Ref<MoveSelectionCommand> Create(Ref<dom::DocumentFragment> &&fragment,
                                                           const dom::Position &position,
                                                           bool smartInsert = false,
                                                           bool smartDelete = false) noexcept;

  private:
    void DoApply() noexcept override;
    KRYS_NODISCARD EditAction EditingAction() const noexcept override;
    KRYS_NODISCARD bool ShouldDispatchInputEvents() const noexcept final
    {
      return false;
    }
  };
}

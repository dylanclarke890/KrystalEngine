#pragma once

#include "Krystal.Booey/Editing/EditCommand.hpp"

namespace krys::boo::dom
{
  class Text;
}

namespace krys::boo
{
  /// @brief Deletes characters from a text node.
  class DeleteFromTextNodeCommand final : public SimpleEditCommand
  {
  private:
    const Ref<dom::Text> _node;
    uint32 _offset;
    uint32 _count;
    utf8_string _text;

  public:
    KRYS_NODISCARD static Ref<DeleteFromTextNodeCommand>
      Create(dom::Text &node, uint32 offset, uint32 count,
             EditAction editingAction = EditAction::Delete) noexcept;

    DeleteFromTextNodeCommand(dom::Text &node, uint32 offset, uint32 count,
                              EditAction editingAction) noexcept;

  private:
    void DoApply() noexcept override;
    void DoUnapply() noexcept override;
  };
}

#pragma once

#include "Krystal.Booey/Editing/EditCommand.hpp"

namespace krys::boo::dom
{
  class Text;
}

namespace krys::boo
{
  /// @brief Inserts characters into a text node.
  class InsertIntoTextNodeCommand final : public SimpleEditCommand
  {
  private:
    const Ref<dom::Text> _node;
    uint32 _offset;
    utf8_string _text;
    AllowPasswordEcho _allowPasswordEcho {AllowPasswordEcho::Yes};

  public:
    KRYS_NODISCARD static Ref<InsertIntoTextNodeCommand>
      Create(dom::Text &node, uint32 offset, utf8_stringview text,
             AllowPasswordEcho allowPasswordEcho = AllowPasswordEcho::Yes,
             EditAction editingAction = EditAction::Insert) noexcept;

    InsertIntoTextNodeCommand(dom::Text &node, uint32 offset, utf8_stringview text,
                              AllowPasswordEcho allowPasswordEcho, EditAction editingAction) noexcept;

    KRYS_NODISCARD const utf8_string &InsertedText() const noexcept
    {
      return _text;
    }

  private:
    void DoApply() noexcept override;
    void DoUnapply() noexcept override;
    void DoReapply() noexcept override;
  };
}

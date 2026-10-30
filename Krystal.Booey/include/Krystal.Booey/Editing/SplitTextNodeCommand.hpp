#pragma once

#include "Krystal.Booey/Editing/EditCommand.hpp"

namespace krys::boo::dom
{
  class Text;
}

namespace krys::boo
{
  /// @brief Splits a text node in two: the new node holds the start, and the original keeps the rest.
  class SplitTextNodeCommand final : public SimpleEditCommand
  {
  private:
    RefPtr<dom::Text> _text1;
    const Ref<dom::Text> _text2;
    uint32 _offset;

  public:
    KRYS_NODISCARD static Ref<SplitTextNodeCommand> Create(dom::Text &node, uint32 offset) noexcept;

    SplitTextNodeCommand(dom::Text &node, uint32 offset) noexcept;

  private:
    void DoApply() noexcept override;
    void DoUnapply() noexcept override;
    void DoReapply() noexcept override;
    void InsertText1AndTrimText2() noexcept;
  };
}

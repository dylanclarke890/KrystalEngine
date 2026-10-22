#pragma once

#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/CompositeEditCommand.hpp"

namespace krys::boo
{
  /// @brief Splits a text node, and the inline element it's in, at an offset.
  class SplitTextNodeContainingElementCommand final : public CompositeEditCommand
  {
  private:
    const Ref<dom::Text> _text;
    uint32 _offset;

  public:
    SplitTextNodeContainingElementCommand(dom::Text &text, uint32 offset) noexcept;

    KRYS_NODISCARD static Ref<SplitTextNodeContainingElementCommand> Create(dom::Text &text,
                                                                            uint32 offset) noexcept;

  private:
    void DoApply() noexcept override;
  };
}

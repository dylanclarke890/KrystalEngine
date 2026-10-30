#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'text-align-last'> = auto | start | end | left | right | center | justify | match-parent
  // NOTE: `match-parent` is computed to a specific alignment during style building.
  // https://drafts.csswg.org/css-text/#propdef-text-align-last
  enum class TextAlignLast : uint8
  {
    Auto,
    Start,
    End,
    Left,
    Right,
    Center,
    Justify
  };

  // MARK: - Conversion

  // NOTE: Custom conversion is required to resolve `match-parent`.
  template <>
  struct CSSValueConversion<TextAlignLast>
  {
    KRYS_NODISCARD TextAlignLast operator()(BuilderState &, const css::Value &) noexcept;
  };
}

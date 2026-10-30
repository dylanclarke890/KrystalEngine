#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'text-align'> = start | end | left | right | center | justify | match-parent | justify-all |
  // -webkit-left | -webkit-right | -webkit-center NOTE: `match-parent` is computed to a specific alignment
  // during style building.
  // FIXME: Support `justify-all`
  // https://drafts.csswg.org/css-text/#propdef-text-align

  // The order of this enum must match the order of the text align values in css::ValueId::Keywords.in.
  enum class TextAlign : uint8
  {
    Left,
    Right,
    Center,
    Justify,
    WebKitLeft,
    WebKitRight,
    WebKitCenter,
    Start,
    End,
  };

  // MARK: - Conversion

  // NOTE: Custom conversion is required to resolve `match-parent` and `-internal-th-center`.
  template <>
  struct CSSValueConversion<TextAlign>
  {
    KRYS_NODISCARD TextAlign operator()(BuilderState &, const css::Value &) noexcept;
  };
}

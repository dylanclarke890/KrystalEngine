#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'resize'> = none | both | horizontal | vertical | block | inline
  // https://drafts.csswg.org/css-ui-4/#propdef-resize

  enum class Resize : uint8
  {
    None,
    Both,
    Horizontal,
    Vertical,
    Block,
    Inline,
  };

  // MARK: - Conversion

  // NOTE: Custom conversion is required to resolve `-internal-textarea-auto`.
  template <>
  struct CSSValueConversion<Resize>
  {
    KRYS_NODISCARD Resize operator()(BuilderState &, const css::Value &) noexcept;
  };
}

#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <single-mask-mode> = alpha | luminance | match-source
  // https://drafts.fxtf.org/css-masking-1/#typedef-masking-mode

  enum class MaskMode : uint8
  {
    Alpha,
    Luminance,
    MatchSource,
  };

  // MARK: - Conversion

  // NOTE: Custom conversion is required to resolve `auto` value when aliased to `-webkit-mask-source-type`.
  template <>
  struct CSSValueConversion<MaskMode>
  {
    KRYS_NODISCARD MaskMode operator()(BuilderState &, const css::Value &) noexcept;
  };
}

#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'max-lines`> = none | <integer [1,∞]>
  // https://www.w3.org/TR/css-overflow-4/#propdef-max-lines.
  struct MaximumLines : css::ValueOrKeyword<Integer<css::Range {1, css::Range::Inf}>, css::keywords::None>
  {
    using Base::Base;

    KRYS_NODISCARD constexpr bool IsNone() const noexcept
    {
      return IsKeyword();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::MaximumLines)

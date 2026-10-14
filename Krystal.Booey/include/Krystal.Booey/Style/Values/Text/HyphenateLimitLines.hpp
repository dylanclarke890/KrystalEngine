#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'hyphenate-limit-lines'> = no-limit | <integer [0,∞]>
  // https://www.w3.org/TR/css-text-4/#propdef-hyphenate-limit-lines
  struct HyphenateLimitLines : css::ValueOrKeyword<Integer<css::NonNegative, short>, css::keywords::NoLimit>
  {
    using Base::Base;

    KRYS_NODISCARD constexpr bool IsNoLimit() const noexcept
    {
      return IsKeyword();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::HyphenateLimitLines)

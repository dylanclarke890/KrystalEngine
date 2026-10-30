#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'-webkit-hyphenate-limit-before'/`-webkit-hyphenate-limit-after`> = auto | <integer [0,∞]>
  // NOTE: There is no standard associated with these specific properties, but there is
  // `hyphenate-limit-chars` which offers a superset of the functionality. See
  // https://www.w3.org/TR/css-text-4/#propdef-hyphenate-limit-chars.
  struct HyphenateLimitEdge : css::ValueOrKeyword<Integer<css::NonNegative, short>, css::keywords::Auto>
  {
    using Base::Base;

    KRYS_NODISCARD constexpr bool IsAuto() const noexcept
    {
      return IsKeyword();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::HyphenateLimitEdge)

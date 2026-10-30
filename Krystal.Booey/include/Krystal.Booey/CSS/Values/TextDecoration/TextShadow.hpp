#pragma once

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Core/Types/Markable.hpp"

namespace krys::boo::css
{
  // <text-shadow> = <color>? && [<length>{2} <length [0,∞]>?]
  // https://drafts.csswg.org/css-text-decor-3/#propdef-text-shadow
  struct TextShadow
  {
    Markable<boo::css::Color> Color;
    SpaceSeparatedPoint<Length<AllUnzoomed>> Location;
    Markable<Length<NonNegativeUnzoomed>> Blur;

    KRYS_NODISCARD bool operator==(const TextShadow &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const TextShadow &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Color;
    }
    else if constexpr (I == 1)
    {
      return value.Location;
    }
    else if constexpr (I == 2)
    {
      return value.Blur;
    }
  }
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::css::TextShadow, 3)

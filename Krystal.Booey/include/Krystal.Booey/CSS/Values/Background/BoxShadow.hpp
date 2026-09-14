#pragma once

#include "Krystal.Booey/CSS/Values/Aggregates.hpp"
#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::css
{
  // <box-shadow> = <color>? && [<length>{2} <length [0,∞]>? <length>?] && inset?
  // https://drafts.csswg.org/css-backgrounds-3/#typedef-shadow
  struct BoxShadow
  {
    Maybe<Color> color;
    SpaceSeparatedPoint<Length<AllUnzoomed>> location;
    Maybe<Length<NonNegativeUnzoomed>> blur;
    Maybe<Length<AllUnzoomed>> spread;
    Maybe<keywords::Inset> inset;
    bool isWebkitBoxShadow;

    KRYS_NODISCARD bool operator==(const BoxShadow &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const BoxShadow &value) noexcept
  {
    if constexpr (!I)
    {
      return value.color;
    }
    else if constexpr (I == 1)
    {
      return value.location;
    }
    else if constexpr (I == 2)
    {
      return value.blur;
    }
    else if constexpr (I == 3)
    {
      return value.spread;
    }
    else if constexpr (I == 4)
    {
      return value.inset;
    }
  }
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::css::BoxShadow, 5)

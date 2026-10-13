#pragma once

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Core/Types/Markable.hpp"

namespace krys::boo::css
{
  // <box-shadow> = <color>? && [<length>{2} <length [0,∞]>? <length>?] && inset?
  // https://drafts.csswg.org/css-backgrounds-3/#typedef-shadow
  struct BoxShadow
  {
    Markable<boo::css::Color> Color;
    SpaceSeparatedPoint<Length<AllUnzoomed>> Location;
    Markable<Length<NonNegativeUnzoomed>> Blur;
    Markable<Length<AllUnzoomed>> Spread;
    Maybe<keywords::Inset> Inset;
    bool IsWebkitBoxShadow {false};

    KRYS_NODISCARD bool operator==(const BoxShadow &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const BoxShadow &value) noexcept
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
    else if constexpr (I == 3)
    {
      return value.Spread;
    }
    else if constexpr (I == 4)
    {
      return value.Inset;
    }
  }
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::css::BoxShadow, 5)

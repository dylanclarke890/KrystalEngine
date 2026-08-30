#pragma once

#include "Krystal.HTML/CSS/Values/Color/Color.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // drop-shadow() = drop-shadow( [ <color>? && [<length>{2} <length [0,∞]>?] ] )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-drop-shadow
  struct DropShadow
  {
    Markable<Color> color;
    SpaceSeparatedPoint<Length<AllUnzoomed>> location;
    Markable<Length<NonNegativeUnzoomed>> stdDeviation;

    bool operator==(const DropShadow &) const = default;
  };

  using DropShadowFunction = FunctionNotation<CSSValueId::DropShadow, DropShadow>;

  template <size_t I>
  KRYS_NODISCARD const auto &get(const DropShadow &value) noexcept
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
      return value.stdDeviation;
    }
  }
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(Krys::HTML::DropShadow, 3)

#pragma once

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  // drop-shadow() = drop-shadow( [ <color>? && [<length>{2} <length [0,∞]>?] ] )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-drop-shadow
  struct DropShadow
  {
    Markable<boo::css::Color> Color;
    SpaceSeparatedPoint<Length<AllUnzoomed>> Location;
    Markable<Length<NonNegativeUnzoomed>> StdDeviation;

    KRYS_NODISCARD bool operator==(const DropShadow &) const noexcept = default;
  };

  using DropShadowFunction = FunctionNotation<ValueId::DropShadow, DropShadow>;

  template <size_t I>
  KRYS_NODISCARD const auto &get(const DropShadow &value) noexcept
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
      return value.StdDeviation;
    }
  }
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::css::DropShadow, 3)

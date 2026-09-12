#pragma once

#include "Krystal.Booey/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace krys::boo::css
{
  // <cubic-bezier()> = cubic-bezier( [ <number [0,1]>, <number> ]#{2} )
  // https://drafts.csswg.org/css-easing-2/#funcdef-cubic-bezier
  struct CubicBezierEasingParameters
  {
    using Coordinate = CommaSeparatedTuple<Number<ClosedUnitRange>, Number<>>;

    CommaSeparatedPair<Coordinate> value;

    constexpr bool operator==(const CubicBezierEasingParameters &) const noexcept = default;
  };

  using CubicBezierEasingFunction = FunctionNotation<CSSValueId::CubicBezier, CubicBezierEasingParameters>;

  DEFINE_TYPE_WRAPPER_GET(CubicBezierEasingParameters, value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::CubicBezierEasingParameters, 1)

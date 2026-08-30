#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
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

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::CubicBezierEasingParameters, 1)

#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // opacity() = opacity( [ <number [0,1(clamp upper)] > | <percentage [0,100(clamp upper)]> ]? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-opacity
  struct Opacity
  {
    using Parameter = NumberOrPercentage<ClosedUnitRangeClampUpper, ClosedPercentageRangeClampUpper>;

    Markable<Parameter> value;

    bool operator==(const Opacity &) const = default;
  };

  using OpacityFunction = FunctionNotation<CSSValueId::Opacity, Opacity>;

  DEFINE_TYPE_WRAPPER_GET(Opacity, value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::Opacity, 1)

#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // grayscale() = grayscale( [ <number [0,1(clamp upper)] > | <percentage [0,100(clamp upper)]> ]? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-grayscale
  struct Grayscale
  {
    using Parameter = NumberOrPercentage<ClosedUnitRangeClampUpper, ClosedPercentageRangeClampUpper>;

    Markable<Parameter> value;

    bool operator==(const Grayscale &) const = default;
  };

  using GrayscaleFunction = FunctionNotation<CSSValueId::Grayscale, Grayscale>;

  DEFINE_TYPE_WRAPPER_GET(Grayscale, value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::Grayscale, 1)

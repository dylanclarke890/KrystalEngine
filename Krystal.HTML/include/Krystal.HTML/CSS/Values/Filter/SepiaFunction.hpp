#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // sepia() = sepia( [ <number [0,1(clamp upper)] > | <percentage [0,100(clamp upper)]> ]? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-sepia
  struct Sepia
  {
    using Parameter = NumberOrPercentage<ClosedUnitRangeClampUpper, ClosedPercentageRangeClampUpper>;

    Markable<Parameter> value;

    bool operator==(const Sepia &) const = default;
  };

  using SepiaFunction = FunctionNotation<CSSValueId::Sepia, Sepia>;

  DEFINE_TYPE_WRAPPER_GET(Sepia, value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::Sepia, 1)

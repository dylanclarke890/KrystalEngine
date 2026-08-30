#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // invert() = invert( [ <number [0,1(clamp upper)] > | <percentage [0,100(clamp upper)]> ]? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-invert
  struct Invert
  {
    using Parameter = NumberOrPercentage<ClosedUnitRangeClampUpper, ClosedPercentageRangeClampUpper>;

    Markable<Parameter> value;

    bool operator==(const Invert &) const = default;
  };

  using InvertFunction = FunctionNotation<CSSValueId::Invert, Invert>;

  DEFINE_TYPE_WRAPPER_GET(Invert, value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::Invert, 1)

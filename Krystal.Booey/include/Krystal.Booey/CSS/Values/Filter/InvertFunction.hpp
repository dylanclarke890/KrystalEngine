#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  // invert() = invert( [ <number [0,1(clamp upper)] > | <percentage [0,100(clamp upper)]> ]? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-invert
  struct Invert
  {
    using Parameter = NumberOrPercentage<ClosedUnitRangeClampUpper, ClosedPercentageRangeClampUpper>;

    Markable<Parameter> value;

    bool operator==(const Invert &) const = default;
  };

  using InvertFunction = FunctionNotation<ValueId::Invert, Invert>;

  DEFINE_TYPE_WRAPPER_GET(Invert, value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Invert, 1)

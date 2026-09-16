#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  // sepia() = sepia( [ <number [0,1(clamp upper)] > | <percentage [0,100(clamp upper)]> ]? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-sepia
  struct Sepia
  {
    using Parameter = NumberOrPercentage<ClosedUnitRangeClampUpper, ClosedPercentageRangeClampUpper>;

    Markable<Parameter> value;

    bool operator==(const Sepia &) const = default;
  };

  using SepiaFunction = FunctionNotation<ValueId::Sepia, Sepia>;

  DEFINE_TYPE_WRAPPER_GET(Sepia, value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Sepia, 1)

#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  // opacity() = opacity( [ <number [0,1(clamp upper)] > | <percentage [0,100(clamp upper)]> ]? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-opacity
  struct Opacity
  {
    using Parameter = NumberOrPercentage<ClosedUnitRangeClampUpper, ClosedPercentageRangeClampUpper>;

    Markable<Parameter> Value;

    KRYS_NODISCARD bool operator==(const Opacity &) const noexcept = default;
  };

  using OpacityFunction = FunctionNotation<ValueId::Opacity, Opacity>;

  DEFINE_TYPE_WRAPPER_GET(Opacity, Value)
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Opacity, 1)

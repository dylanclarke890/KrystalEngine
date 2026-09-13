#pragma once

#include "Krystal.Booey/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace krys::boo::css
{
  // saturate() = saturate( [ <number [0,∞]> | <percentage [0,∞]> ]? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-saturate
  struct Saturate
  {
    using Parameter = NumberOrPercentage<NonNegative>;

    Markable<Parameter> value;

    bool operator==(const Saturate &) const = default;
  };

  using SaturateFunction = FunctionNotation<ValueId::Saturate, Saturate>;

  DEFINE_TYPE_WRAPPER_GET(Saturate, value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Saturate, 1)

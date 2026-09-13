#pragma once

#include "Krystal.Booey/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace krys::boo::css
{
  // brightness() = brightness( [ <number [0,∞]> | <percentage [0,∞]> ]? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-brightness
  struct Brightness
  {
    using Parameter = NumberOrPercentage<NonNegative>;

    Markable<Parameter> value;

    bool operator==(const Brightness &) const = default;
  };

  using BrightnessFunction = FunctionNotation<ValueId::Brightness, Brightness>;

  DEFINE_TYPE_WRAPPER_GET(Brightness, value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Brightness, 1)

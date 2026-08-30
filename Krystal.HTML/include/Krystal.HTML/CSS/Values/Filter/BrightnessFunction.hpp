#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // brightness() = brightness( [ <number [0,∞]> | <percentage [0,∞]> ]? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-brightness
  struct Brightness
  {
    using Parameter = NumberOrPercentage<NonNegative>;

    Markable<Parameter> value;

    bool operator==(const Brightness &) const = default;
  };

  using BrightnessFunction = FunctionNotation<CSSValueId::Brightness, Brightness>;

  DEFINE_TYPE_WRAPPER_GET(Brightness, value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::Brightness, 1)

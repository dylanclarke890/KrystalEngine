#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // saturate() = saturate( [ <number [0,∞]> | <percentage [0,∞]> ]? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-saturate
  struct Saturate
  {
    using Parameter = NumberOrPercentage<NonNegative>;

    Markable<Parameter> value;

    bool operator==(const Saturate &) const = default;
  };

  using SaturateFunction = FunctionNotation<CSSValueId::Saturate, Saturate>;

  DEFINE_TYPE_WRAPPER_GET(Saturate, value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::Saturate, 1)

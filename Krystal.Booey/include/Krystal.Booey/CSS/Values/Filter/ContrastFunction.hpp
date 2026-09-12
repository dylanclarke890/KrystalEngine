#pragma once

#include "Krystal.Booey/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace krys::boo::css
{
  // contrast() = contrast( [ <number [0,∞]> | <percentage [0,∞]> ]? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-contrast
  struct Contrast
  {
    using Parameter = NumberOrPercentage<NonNegative>;

    Markable<Parameter> value;

    bool operator==(const Contrast &) const = default;
  };

  using ContrastFunction = FunctionNotation<CSSValueId::Contrast, Contrast>;

  DEFINE_TYPE_WRAPPER_GET(Contrast, value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Contrast, 1)

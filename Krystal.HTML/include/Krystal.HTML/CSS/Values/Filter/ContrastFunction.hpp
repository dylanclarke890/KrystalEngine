#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
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

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::Contrast, 1)

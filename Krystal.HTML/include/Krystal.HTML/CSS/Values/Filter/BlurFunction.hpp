#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // blur() = blur( <length [0,∞]>? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-blur
  struct Blur
  {
    using Parameter = Length<NonNegative>;

    Markable<Parameter> value;

    bool operator==(const Blur &) const = default;
  };

  using BlurFunction = FunctionNotation<CSSValueId::Blur, Blur>;

  DEFINE_TYPE_WRAPPER_GET(Blur, value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::Blur, 1)

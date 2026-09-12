#pragma once

#include "Krystal.Booey/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace krys::boo::css
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

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Blur, 1)

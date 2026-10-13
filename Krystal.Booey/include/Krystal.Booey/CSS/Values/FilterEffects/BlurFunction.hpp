#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  // blur() = blur( <length [0,∞]>? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-blur
  struct Blur
  {
    using Parameter = Length<NonNegative>;

    Markable<Parameter> Value;

    KRYS_NODISCARD bool operator==(const Blur &) const noexcept = default;
  };

  using BlurFunction = FunctionNotation<ValueId::Blur, Blur>;

  DEFINE_TYPE_WRAPPER_GET(Blur, Value)
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Blur, 1)

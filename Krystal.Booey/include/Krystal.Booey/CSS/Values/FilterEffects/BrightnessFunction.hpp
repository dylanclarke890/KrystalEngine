#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  // brightness() = brightness( [ <number [0,∞]> | <percentage [0,∞]> ]? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-brightness
  struct Brightness
  {
    using Parameter = NumberOrPercentage<NonNegative>;

    Markable<Parameter> Value;

    KRYS_NODISCARD bool operator==(const Brightness &) const noexcept = default;
  };

  using BrightnessFunction = FunctionNotation<ValueId::Brightness, Brightness>;

  DEFINE_TYPE_WRAPPER_GET(Brightness, Value)
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Brightness, 1)

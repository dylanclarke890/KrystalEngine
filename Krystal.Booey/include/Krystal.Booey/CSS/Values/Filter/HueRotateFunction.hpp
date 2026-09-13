#pragma once

#include "Krystal.Booey/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace krys::boo::css
{
  // hue-rotate() = hue-rotate( [ <angle> | <zero> ]? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-hue-rotate
  struct HueRotate
  {
    using Parameter = Angle<>;

    Markable<Parameter> value;

    bool operator==(const HueRotate &) const = default;
  };

  using HueRotateFunction = FunctionNotation<ValueId::HueRotate, HueRotate>;

  DEFINE_TYPE_WRAPPER_GET(HueRotate, value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::HueRotate, 1)

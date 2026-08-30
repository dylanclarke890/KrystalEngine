#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // hue-rotate() = hue-rotate( [ <angle> | <zero> ]? )
  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-hue-rotate
  struct HueRotate
  {
    using Parameter = Angle<>;

    Markable<Parameter> value;

    bool operator==(const HueRotate &) const = default;
  };

  using HueRotateFunction = FunctionNotation<CSSValueId::HueRotate, HueRotate>;

  DEFINE_TYPE_WRAPPER_GET(HueRotate, value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::HueRotate, 1)

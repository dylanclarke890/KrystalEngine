#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  // <'r'> = <length-percentage [0,∞]>
  // https://svgwg.org/svg2-draft/geometry.html#R
  struct SVGRadius : LengthWrapperBase<LengthPercentage<css::NonNegative>>
  {
    using Base::Base;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SVGRadius)

#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  // <'x'/'y'> = <length-percentage>
  // https://svgwg.org/svg2-draft/geometry.html#X
  // https://svgwg.org/svg2-draft/geometry.html#Y
  struct SVGCoordinateComponent : LengthWrapperBase<LengthPercentage<>>
  {
    using Base::Base;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SVGCoordinateComponent)

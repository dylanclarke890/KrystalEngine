#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  // <'cx'/'cy'> = <length-percentage>
  // https://svgwg.org/svg2-draft/geometry.html#CX
  // https://svgwg.org/svg2-draft/geometry.html#CY
  struct SVGCenterCoordinateComponent : LengthWrapperBase<LengthPercentage<>>
  {
    using Base::Base;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SVGCenterCoordinateComponent)

#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  // <'rx'/'ry'> = <length-percentage [0,∞]> | auto
  // https://svgwg.org/svg2-draft/geometry.html#RX
  // https://svgwg.org/svg2-draft/geometry.html#RY
  struct SVGRadiusComponent : LengthWrapperBase<LengthPercentage<css::NonNegative>, css::keywords::Auto>
  {
    using Base::Base;

    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return HoldsAlternative<css::keywords::Auto>();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SVGRadiusComponent)

#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  // <'shape-margin'> = <length-percentage [0,∞]>
  // https://www.w3.org/TR/css-shapes/#propdef-shape-margin
  struct ShapeMargin : LengthWrapperBase<LengthPercentage<css::NonNegative>>
  {
    using Base::Base;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ShapeMargin)

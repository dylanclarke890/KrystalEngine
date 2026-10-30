#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  // <'shape-image-threshold'> = <number>
  // FIXME: Current spec has this defined as <opacity-value> (e.g. <number> | <percentage>).
  // https://www.w3.org/TR/css-shapes/#propdef-shape-image-threshold
  using ShapeImageThreshold = Number<css::ClosedUnitRangeClampBoth, float>;
}

#pragma once

#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"

namespace krys::boo::style
{
  // <'flex-shrink'> = <number [0,∞]>
  // https://drafts.csswg.org/css-flexbox/#propdef-flex-shrink
  using FlexShrink = Number<css::NonNegative, float>;
}

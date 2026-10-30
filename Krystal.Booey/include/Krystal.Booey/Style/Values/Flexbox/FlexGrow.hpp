#pragma once

#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"

namespace krys::boo::style
{
  // <'flex-grow'> = <number [0,∞]>
  // https://drafts.csswg.org/css-flexbox/#propdef-flex-grow
  using FlexGrow = Number<css::NonNegative, float>;
}

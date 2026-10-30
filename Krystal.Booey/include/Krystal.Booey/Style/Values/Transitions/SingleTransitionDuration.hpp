#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  // <single-transition-duration> = <time [0,∞]>
  // https://www.w3.org/TR/css-transitions-1/#propdef-transition-duration
  using SingleTransitionDuration = Time<css::NonNegative>;
}

#pragma once

#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"

namespace krys::boo::style
{
  // <'-webkit-border-horizontal-spacing'/'-webkit-border-vertical-spacing'> = <length [0,∞]>
  // NOTE: There is no standard associated with this property.
  using WebkitBorderSpacing = Length<css::NonNegativeUnzoomed>;
}

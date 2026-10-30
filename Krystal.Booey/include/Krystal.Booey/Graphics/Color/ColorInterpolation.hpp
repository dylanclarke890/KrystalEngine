#pragma once

#include "Krystal.Core/Colour/ColourInterpolation.hpp"

namespace krys::boo
{
  class Color;

  Color InterpolateColors(ColourInterpolationMethod, Color color1, double color1Multiplier, Color color2,
                          double color2Multiplier) noexcept;
}

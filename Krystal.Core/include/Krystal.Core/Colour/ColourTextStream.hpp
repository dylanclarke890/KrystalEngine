#pragma once

#include "Krystal.Core/Colour/AlphaPremultiplication.hpp"
#include "Krystal.Core/Colour/BlendMode.hpp"
#include "Krystal.Core/Colour/ColourInterpolationMethod.hpp"
#include "Krystal.Core/Colour/GradientSpreadMethod.hpp"

namespace krys
{
  class TextStream;

  TextStream &operator<<(TextStream &ts, AlphaPremultiplication premultiplication) noexcept;
  TextStream &operator<<(TextStream &ts, BlendMode blendMode) noexcept;
  TextStream &operator<<(TextStream &ts, GradientSpreadMethod spreadMethod) noexcept;
  TextStream &operator<<(TextStream &ts, ColourInterpolationColourSpace interpolationColorSpace) noexcept;
  TextStream &operator<<(TextStream &ts, HueInterpolationMethod hueInterpolationMethod) noexcept;
  TextStream &operator<<(TextStream &ts, const ColourInterpolationMethod &method) noexcept;
}

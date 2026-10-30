#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#canvastextrendering
  enum class CanvasTextRendering : uint8
  {
    Auto,
    OptimizeSpeed,
    OptimizeLegibility,
    GeometricPrecision
  };
}

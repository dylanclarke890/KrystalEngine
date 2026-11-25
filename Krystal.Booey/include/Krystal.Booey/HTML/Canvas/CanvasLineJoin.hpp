#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/multipage/canvas.html#canvaslinejoin
  enum class CanvasLineJoin : uint8
  {
    Round,
    Bevel,
    Miter,
  };
}

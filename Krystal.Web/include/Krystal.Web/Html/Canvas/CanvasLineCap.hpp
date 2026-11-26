#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::web::html
{
  /// @see https://html.spec.whatwg.org/multipage/canvas.html#canvaslinecap
  enum class CanvasLineCap : uint8
  {
    Butt,
    Round,
    Square,
  };
}

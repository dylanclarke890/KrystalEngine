#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::web::html
{
  /// @see https://html.spec.whatwg.org/multipage/canvas.html#canvasfillrule
  enum class CanvasFillRule : uint8
  {
    Nonzero,
    Evenodd,
  };
}

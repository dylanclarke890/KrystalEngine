#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::web::html
{
  /// @see https://html.spec.whatwg.org/multipage/canvas.html#canvasdirection
  enum class CanvasDirection : uint8
  {
    Ltr,
    Rtl,
    Inherit,
  };
}

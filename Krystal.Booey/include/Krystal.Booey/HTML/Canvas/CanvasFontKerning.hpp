#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/multipage/canvas.html#canvasfontkerning
  enum class CanvasFontKerning : uint8
  {
    Auto,
    Normal,
    None,
  };
}

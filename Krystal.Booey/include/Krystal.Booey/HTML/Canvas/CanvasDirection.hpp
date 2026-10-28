#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#canvasdirection
  enum class CanvasDirection : uint8
  {
    Ltr,
    Rtl,
    Inherit
  };
}

#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#canvastextalign
  enum class CanvasTextAlign : uint8
  {
    Start,
    End,
    Left,
    Right,
    Center
  };
}

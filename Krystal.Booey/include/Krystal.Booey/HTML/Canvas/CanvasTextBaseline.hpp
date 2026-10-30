#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#canvastextbaseline
  enum class CanvasTextBaseline : uint8
  {
    Top,
    Hanging,
    Middle,
    Alphabetic,
    Ideographic,
    Bottom
  };
}

#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/multipage/canvas.html#imagesmoothingquality
  enum class ImageSmoothingQuality : uint8
  {
    Low,
    Medium,
    High,
  };
}

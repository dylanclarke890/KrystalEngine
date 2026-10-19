#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief A colour at an offset (from 0 to 1) along a gradient.
  struct GradientColorStop
  {
    float Offset {0};
    boo::Color Color {};
  };
}

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, const GradientColorStop &stop) noexcept;
}

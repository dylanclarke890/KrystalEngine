#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief The direction an arc is drawn in.
  enum class RotationDirection : bool
  {
    Counterclockwise,
    Clockwise
  };
}

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, RotationDirection direction) noexcept;
}

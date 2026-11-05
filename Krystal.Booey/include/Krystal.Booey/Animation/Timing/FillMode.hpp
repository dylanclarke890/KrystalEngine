#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief Whether an animation effect applies before and after its active interval.
  /// @see https://drafts.csswg.org/web-animations-1/#fill-behavior
  enum class FillMode : uint8
  {
    None,
    Forwards,
    Backwards,
    Both,
    Auto
  };
}

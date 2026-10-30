#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief The direction an animation effect's iterations play in.
  /// @see https://drafts.csswg.org/web-animations-1/#direction-control
  enum class PlaybackDirection : uint8
  {
    Normal,
    Reverse,
    Alternate,
    AlternateReverse
  };
}

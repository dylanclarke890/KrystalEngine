#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::web
{
  /// @see https://drafts.csswg.org/web-animations-1/#animation-effect-phases-and-states
  enum class AnimationEffectPhase : uint8
  {
    Before,
    Active,
    After,
    Idle,
  };
}

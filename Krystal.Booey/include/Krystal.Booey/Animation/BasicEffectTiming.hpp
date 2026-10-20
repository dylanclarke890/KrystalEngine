#pragma once

#include "Krystal.Booey/Animation/AnimationEffectPhase.hpp"
#include "Krystal.Booey/Animation/WebAnimationTime.hpp"

namespace krys::boo
{
  /// @brief An animation effect's local time, active time, end time, active duration and phase.
  struct BasicEffectTiming
  {
    Maybe<WebAnimationTime> LocalTime;
    Maybe<WebAnimationTime> ActiveTime;
    WebAnimationTime EndTime;
    WebAnimationTime ActiveDuration;
    AnimationEffectPhase Phase;
  };
}

#pragma once

#include "Krystal.Booey/Animation/AnimationEffectPhase.hpp"
#include "Krystal.Booey/Animation/EffectTiming.hpp"
#include "Krystal.Booey/Animation/WebAnimationTime.hpp"
#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"

namespace krys::boo
{
  /// @brief An animation effect's timing at its current time.
  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-computedeffecttiming
  struct ComputedEffectTiming : EffectTiming
  {
    AnimationEffectPhase Phase {AnimationEffectPhase::Idle};
    Maybe<WebAnimationTime> LocalTime;
    Maybe<double> SimpleIterationProgress;
    Maybe<double> Progress;
    Maybe<double> CurrentIteration;
    WebAnimationTime EndTime;
    WebAnimationTime ActiveDuration;
    TimingFunction::Before Before {TimingFunction::Before::No};
  };
}

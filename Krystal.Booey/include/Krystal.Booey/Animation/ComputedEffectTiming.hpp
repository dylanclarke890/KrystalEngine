#pragma once

#include "Krystal.Booey/Animation/AnimationEffectPhase.hpp"
#include "Krystal.Booey/Animation/FillMode.hpp"
#include "Krystal.Booey/Animation/PlaybackDirection.hpp"
#include "Krystal.Booey/Animation/WebAnimationTime.hpp"
#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"

namespace krys::boo
{
  /// @brief An animation effect's timing at its current time.
  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-computedeffecttiming
  // FIXME: The `EffectTiming` members with their script types (the duration as a number or a percentage, the
  // easing as a string), once there are the Web Animations interfaces.
  struct ComputedEffectTiming
  {
    /// @brief In milliseconds.
    double Delay {0};
    /// @brief In milliseconds.
    double EndDelay {0};
    double IterationStart {0};
    double Iterations {1};
    /// @brief The iteration duration, or the intrinsic iteration duration for an `auto` duration.
    WebAnimationTime Duration;
    FillMode Fill {FillMode::Auto};
    PlaybackDirection Direction {PlaybackDirection::Normal};

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

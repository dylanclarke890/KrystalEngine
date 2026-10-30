#pragma once

#include "Krystal.Booey/Animation/BasicEffectTiming.hpp"
#include "Krystal.Booey/Animation/FillMode.hpp"
#include "Krystal.Booey/Animation/PlaybackDirection.hpp"
#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"

namespace krys::boo
{
  /// @brief An animation effect's progress at a time.
  struct ResolvedEffectTiming
  {
    Maybe<double> CurrentIteration;
    AnimationEffectPhase Phase {AnimationEffectPhase::Idle};
    Maybe<double> TransformedProgress;
    Maybe<double> SimpleIterationProgress;
    TimingFunction::Before Before;
  };

  /// @brief An animation effect's timing properties, specified and computed, and the timing model's
  /// calculations.
  /// @see https://drafts.csswg.org/web-animations-2/#timing-model
  struct AnimationEffectTiming
  {
    RefPtr<boo::TimingFunction> TimingFunction {LinearTimingFunction::Create()};
    FillMode Fill {FillMode::Auto};
    PlaybackDirection Direction {PlaybackDirection::Normal};
    double IterationStart {0};
    double Iterations {1};
    WebAnimationTime StartDelay {Seconds {0}};
    WebAnimationTime EndDelay {Seconds {0}};
    WebAnimationTime IterationDuration {Seconds {0}};
    WebAnimationTime IntrinsicIterationDuration {Seconds {0}};
    WebAnimationTime ActiveDuration {Seconds {0}};
    WebAnimationTime EndTime {Seconds {0}};

    Seconds SpecifiedStartDelay {0};
    Seconds SpecifiedEndDelay {0};
    Maybe<Seconds> SpecifiedIterationDuration;

    struct ResolutionData
    {
      Maybe<WebAnimationTime> TimelineTime;
      Maybe<WebAnimationTime> TimelineDuration;
      Maybe<WebAnimationTime> StartTime;
      Maybe<WebAnimationTime> LocalTime;
      boo::EndpointInclusiveActiveInterval EndpointInclusiveActiveInterval {
        boo::EndpointInclusiveActiveInterval::No};
      double PlaybackRate {0};
    };

    /// @brief Computes the delays, iteration duration, active duration and end time from the specified ones.
    void UpdateComputedProperties(Maybe<WebAnimationTime> timelineDuration, double playbackRate) noexcept;
    KRYS_NODISCARD BasicEffectTiming GetBasicTiming(const ResolutionData &data) const noexcept;
    KRYS_NODISCARD ResolvedEffectTiming Resolve(const ResolutionData &data) const noexcept;
  };
}

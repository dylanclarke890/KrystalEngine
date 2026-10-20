#include "Krystal.Booey/Animation/AnimationEffectTiming.hpp"
#include "Krystal.Core/Assert.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace krys::boo
{
  void AnimationEffectTiming::UpdateComputedProperties(Maybe<WebAnimationTime> timelineDuration,
                                                       double playbackRate) noexcept
  {
    auto specifiedEndTime = [&]
    {
      krys_debug_assert(SpecifiedIterationDuration);
      auto specifiedRepeatedDuration = *SpecifiedIterationDuration * Iterations;
      auto specifiedActiveDuration = playbackRate ? specifiedRepeatedDuration / std::abs(playbackRate)
                                                  : Seconds {std::numeric_limits<double>::infinity()};
      return std::max(SpecifiedStartDelay + specifiedActiveDuration + SpecifiedEndDelay, Seconds {0});
    };

    auto computeIntrinsicIterationDuration = [&]
    {
      // https://drafts.csswg.org/web-animations-2/#intrinsic-iteration-duration
      if (!timelineDuration || !Iterations)
      {
        // If timeline duration is unresolved or iteration count is zero, return 0
        IntrinsicIterationDuration =
          timelineDuration ? timelineDuration->MatchingZero() : WebAnimationTime::FromMilliseconds(0);
      }
      else
      {
        // Otherwise, return (100% - start delay - end delay) / iteration count
        if (std::isinf(Iterations))
        {
          IntrinsicIterationDuration = WebAnimationTime::FromPercentage(0);
        }
        else
        {
          IntrinsicIterationDuration = (*timelineDuration - StartDelay - EndDelay) / Iterations;
        }
      }
    };

    // https://drafts.csswg.org/web-animations-2/#normalize-specified-timing
    if (timelineDuration)
    {
      // If timeline duration is resolved:
      // Follow the procedure to convert a time-based animation to a proportional animation.
      if (!SpecifiedIterationDuration)
      {
        // If the iteration duration is auto, then perform the following steps.
        // Set start delay and end delay to 0, as it is not possible to mix time and proportions.
        StartDelay = WebAnimationTime::FromPercentage(0);
        EndDelay = WebAnimationTime::FromPercentage(0);
        IterationDuration =
          std::isinf(Iterations) ? WebAnimationTime::FromPercentage(0) : *timelineDuration / Iterations;
      }
      else if (auto totalTime = specifiedEndTime(); totalTime.count())
      {
        auto sanitize = [&](const WebAnimationTime &time)
        {
          if (time.IsInfinity() || time.IsNaN())
          {
            return *timelineDuration;
          }
          return time;
        };
        // Otherwise:
        // Let total time be equal to end time
        // Set start delay to be the result of evaluating specified start delay / total time * timeline
        // duration.
        StartDelay = sanitize(*timelineDuration * (SpecifiedStartDelay / totalTime));
        // Set iteration duration to be the result of evaluating specified iteration duration / total time *
        // timeline duration.
        IterationDuration = sanitize(*timelineDuration * (*SpecifiedIterationDuration / totalTime));
        // Set end delay to be the result of evaluating specified end delay / total time * timeline duration.
        EndDelay = sanitize(*timelineDuration * (SpecifiedEndDelay / totalTime));
      }
      else
      {
        // The spec does not call out this case, filed https://github.com/w3c/csswg-drafts/issues/11276.
        StartDelay = WebAnimationTime::FromPercentage(0);
        EndDelay = WebAnimationTime::FromPercentage(0);
        IterationDuration = WebAnimationTime::FromPercentage(0);
      }
      computeIntrinsicIterationDuration();
    }
    else
    {
      // Otherwise:
      // Set start delay = specified start delay
      StartDelay = SpecifiedStartDelay;
      // Set end delay = specified end delay
      EndDelay = SpecifiedEndDelay;

      computeIntrinsicIterationDuration();

      // If iteration duration is auto:
      // Set iteration duration = intrinsic iteration duration
      // Otherwise:
      // Set iteration duration = specified iteration duration
      IterationDuration = SpecifiedIterationDuration ? WebAnimationTime(*SpecifiedIterationDuration)
                                                     : IntrinsicIterationDuration;
    }

    // https://drafts.csswg.org/web-animations-2/#repeated-duration
    // In order to calculate the active duration we first define the repeated duration as follows:
    //     repeated duration = iteration duration × iteration count
    // If either the iteration duration or iteration count are zero, the repeated duration is zero.
    auto repeatedDuration = [&]
    {
      if (IterationDuration.IsZero() || !Iterations)
      {
        return IterationDuration.MatchingZero();
      }
      return IterationDuration * Iterations;
    };

    // 3.7.2. Calculating the active duration
    // https://drafts.csswg.org/web-animations-2/#calculating-the-active-duration
    ActiveDuration = [&]
    {
      if (IterationDuration.Time())
      {
        return repeatedDuration();
      }

      krys_debug_assert(IterationDuration.Percentage());
      if (std::isinf(Iterations))
      {
        return IterationDuration;
      }
      // The active duration is calculated according to the following steps:
      // If the playback rate is zero, return Infinity.
      if (!playbackRate)
      {
        return IterationDuration.MatchingInfinity();
      }
      // Otherwise, return repeated duration / abs(playback rate).
      return repeatedDuration() / std::abs(playbackRate);
    }();

    // https://drafts.csswg.org/web-animations-2/#end-time
    // The end time of an animation effect is the result of evaluating
    // max(start time + start delay + active duration + end delay, 0).
    EndTime = std::max(StartDelay + ActiveDuration + EndDelay, ActiveDuration.MatchingZero());
  }

  BasicEffectTiming AnimationEffectTiming::GetBasicTiming(const ResolutionData &data) const noexcept
  {
    // The Web Animations spec introduces a number of animation effect time-related definitions that refer
    // to each other a fair bit, so rather than implementing them as individual methods, it's more efficient
    // to return them all as a single BasicEffectTiming.

    auto localTime = data.LocalTime;

    auto phase = [this, &data, &localTime]() -> AnimationEffectPhase
    {
      // 3.5.5. Animation effect phases and states
      // https://drafts.csswg.org/web-animations-2/#animation-effect-phases-and-states

      // (This should be the last statement, but it's more efficient to cache the local time and return right
      // away if it's not resolved.) Furthermore, it is often convenient to refer to the case when an
      // animation effect is in none of the above phases as being in the idle phase.
      if (!localTime)
      {
        return AnimationEffectPhase::Idle;
      }

      auto atProgressTimelineBoundary = [&]()
      {
        // https://drafts.csswg.org/web-animations-2/#at-progress-timeline-boundary
        // If any of the following conditions are true:
        // - the associated animation's timeline is not a progress-based timeline, or
        // - the associated animation's timeline duration is unresolved or zero, or
        // - the animation’s playback rate is zero
        // return false
        if (!data.TimelineDuration || data.TimelineDuration->IsZero())
        {
          return false;
        }
        if (!data.PlaybackRate)
        {
          return false;
        }
        // Let effective start time be the animation’s start time if resolved, or zero otherwise.
        auto effectiveStartTime = data.StartTime.value_or(WebAnimationTime::FromPercentage(0));
        // Set unlimited current time based on the first matching condition:
        // - start time is resolved: (timeline time - start time) × playback rate
        // - Otherwise: animation's current time
        auto unlimitedCurrentTime = (data.StartTime && data.TimelineTime)
                                      ? (*data.TimelineTime - *data.StartTime) * data.PlaybackRate
                                      : *data.LocalTime;
        // Let effective timeline time be unlimited current time / animation’s playback rate + effective start
        // time
        auto effectiveTimelineTime = unlimitedCurrentTime / data.PlaybackRate + effectiveStartTime;
        // Let effective timeline progress be effective timeline time / timeline duration
        auto effectiveTimelineProgress = effectiveTimelineTime / *data.TimelineDuration;
        // If effective timeline progress is 0 or 1, return true, otherwise false.
        return !effectiveTimelineProgress || effectiveTimelineProgress == 1;
      };

      auto animationIsBackwards = data.PlaybackRate < 0;

      // https://drafts.csswg.org/web-animations-1/#before-active-boundary-time
      auto beforeActiveBoundaryTime = std::max(std::min(StartDelay, EndTime), EndTime.MatchingZero());

      // An animation effect is in the before phase if the animation effect's local time is not unresolved and
      // either of the following conditions are met:
      //     1. the local time is less than the before-active boundary time, or
      //     2. the animation direction is "backwards", the endpoint-inclusive active interval flag is false,
      //        and the local time is equal to the before-active boundary time and not at progress timeline
      //        boundary.
      if (localTime->ApproximatelyLessThan(beforeActiveBoundaryTime)
          || (animationIsBackwards
              && data.EndpointInclusiveActiveInterval == EndpointInclusiveActiveInterval::No
              && localTime->ApproximatelyEqualTo(beforeActiveBoundaryTime) && !atProgressTimelineBoundary()))
      {
        return AnimationEffectPhase::Before;
      }

      // https://drafts.csswg.org/web-animations-1/#active-after-boundary-time
      auto activeAfterBoundaryTime =
        std::max(std::min(StartDelay + ActiveDuration, EndTime), EndTime.MatchingZero());

      // An animation effect is in the after phase if the animation effect's local time is not unresolved
      // and either of the following conditions are met:
      //     1. the local time is greater than the active-after boundary time, or
      //     2. the animation direction is "forwards", the endpoint-inclusive active interval flag is false,
      //        and the local time is equal to the active-after boundary time and not at progress timeline
      //        boundary.
      if (localTime->ApproximatelyGreaterThan(activeAfterBoundaryTime)
          || (!animationIsBackwards
              && data.EndpointInclusiveActiveInterval == EndpointInclusiveActiveInterval::No
              && localTime->ApproximatelyEqualTo(activeAfterBoundaryTime) && !atProgressTimelineBoundary()))
      {
        return AnimationEffectPhase::After;
      }

      // An animation effect is in the active phase if the animation effect’s local time is not unresolved and
      // it is not in either the before phase nor the after phase. (No need to check, we've already
      // established that local time was resolved).
      return AnimationEffectPhase::Active;
    }();

    auto activeTime = [this, &localTime, phase]() -> Maybe<WebAnimationTime>
    {
      // 3.8.3.1. Calculating the active time
      // https://drafts.csswg.org/web-animations-1/#calculating-the-active-time

      // The active time is based on the local time and start delay. However, it is only defined
      // when the animation effect should produce an output and hence depends on its fill mode
      // and phase as follows,

      // If the animation effect is in the before phase, the result depends on the first matching
      // condition from the following,
      if (phase == AnimationEffectPhase::Before)
      {
        // If the fill mode is backwards or both, return the result of evaluating
        // max(local time - start delay, 0).
        if (Fill == FillMode::Backwards || Fill == FillMode::Both)
        {
          return std::max(*localTime - StartDelay, localTime->MatchingZero());
        }
        // Otherwise, return an unresolved time value.
        return null;
      }

      // If the animation effect is in the active phase, return the result of evaluating local time - start
      // delay.
      if (phase == AnimationEffectPhase::Active)
      {
        return *localTime - StartDelay;
      }

      // If the animation effect is in the after phase, the result depends on the first matching
      // condition from the following,
      if (phase == AnimationEffectPhase::After)
      {
        // If the fill mode is forwards or both, return the result of evaluating
        // max(min(local time - start delay, active duration), 0).
        if (Fill == FillMode::Forwards || Fill == FillMode::Both)
        {
          return std::max(std::min(*localTime - StartDelay, ActiveDuration), localTime->MatchingZero());
        }
        // Otherwise, return an unresolved time value.
        return null;
      }

      // Otherwise (the local time is unresolved), return an unresolved time value.
      return null;
    }();

    return {localTime, activeTime, EndTime, ActiveDuration, phase};
  }

  namespace
  {
    enum class ComputedDirection : uint8
    {
      Forwards,
      Reverse
    };
  }

  ResolvedEffectTiming AnimationEffectTiming::Resolve(const ResolutionData &data) const noexcept
  {
    // The Web Animations spec introduces a number of animation effect time-related definitions that refer
    // to each other a fair bit, so rather than implementing them as individual methods, it's more efficient
    // to return them all as a single ComputedEffectTiming.

    auto basicEffectTiming = GetBasicTiming(data);
    auto activeTime = basicEffectTiming.ActiveTime;
    auto phase = basicEffectTiming.Phase;

    auto overallProgress = [this, phase, &activeTime]() -> Maybe<double>
    {
      // 3.8.3.2. Calculating the overall progress
      // https://drafts.csswg.org/web-animations-1/#calculating-the-overall-progress

      // The overall progress describes the number of iterations that have completed (including partial
      // iterations) and is defined as follows:

      // 1. If the active time is unresolved, return unresolved.
      if (!activeTime)
      {
        return null;
      }

      // 2. Calculate an initial value for overall progress based on the first matching condition from below,
      auto overallProgress = [&]()
      {
        // If the iteration duration is zero, if the animation effect is in the before phase, let overall
        // progress be zero, otherwise, let it be equal to the iteration count.
        if (IterationDuration.IsZero())
        {
          return phase == AnimationEffectPhase::Before ? 0 : Iterations;
        }
        // Otherwise, let overall progress be the result of calculating active time / iteration duration.
        return *activeTime / IterationDuration;
      }();

      // 3. Return the result of calculating overall progress + iteration start.
      overallProgress += IterationStart;
      return std::abs(overallProgress);
    }();

    auto simpleIterationProgress = [this, &overallProgress, phase, &activeTime]() -> Maybe<double>
    {
      // 3.8.3.3. Calculating the simple iteration progress
      // https://drafts.csswg.org/web-animations-1/#calculating-the-simple-iteration-progress

      // The simple iteration progress is a fraction of the progress through the current iteration that
      // ignores transformations to the time introduced by the playback direction or timing functions
      // applied to the effect, and is calculated as follows:

      // 1. If the overall progress is unresolved, return unresolved.
      if (!overallProgress)
      {
        return null;
      }

      // 2. If overall progress is infinity, let the simple iteration progress be iteration start % 1.0,
      // otherwise, let the simple iteration progress be overall progress % 1.0.
      double simpleIterationProgress =
        std::isinf(*overallProgress) ? std::fmod(IterationStart, 1) : std::fmod(*overallProgress, 1);

      // 3. If all of the following conditions are true,
      //
      // the simple iteration progress calculated above is zero, and
      // the animation effect is in the active phase or the after phase, and
      // the active time is equal to the active duration, and
      // the iteration count is not equal to zero.
      // let the simple iteration progress be 1.0.
      if (!simpleIterationProgress
          && (phase == AnimationEffectPhase::Active || phase == AnimationEffectPhase::After)
          && activeTime->ApproximatelyEqualTo(ActiveDuration) && Iterations)
      {
        return 1;
      }

      return simpleIterationProgress;
    }();

    auto currentIteration = [this, &activeTime, phase, &simpleIterationProgress,
                             &overallProgress]() -> Maybe<double>
    {
      // 3.8.4. Calculating the current iteration
      // https://drafts.csswg.org/web-animations-1/#calculating-the-current-iteration

      // The current iteration can be calculated using the following steps:

      // 1. If the active time is unresolved, return unresolved.
      if (!activeTime)
      {
        return null;
      }

      // 2. If the animation effect is in the after phase and the iteration count is infinity, return
      // infinity.
      if (phase == AnimationEffectPhase::After && std::isinf(Iterations))
      {
        return std::numeric_limits<double>::infinity();
      }

      // 3. If the simple iteration progress is 1.0, return floor(overall progress) - 1.
      if (*simpleIterationProgress == 1)
      {
        return std::floor(*overallProgress) - 1;
      }

      // 4. Otherwise, return floor(overall progress).
      return std::floor(*overallProgress);
    }();

    auto currentDirection = [this, &currentIteration]() -> ComputedDirection
    {
      // 3.9.1. Calculating the directed progress
      // https://drafts.csswg.org/web-animations-1/#calculating-the-directed-progress

      // If playback direction is normal, let the current direction be forwards.
      if (Direction == PlaybackDirection::Normal)
      {
        return ComputedDirection::Forwards;
      }

      // If playback direction is reverse, let the current direction be reverse.
      if (Direction == PlaybackDirection::Reverse)
      {
        return ComputedDirection::Reverse;
      }

      if (!currentIteration)
      {
        return ComputedDirection::Forwards;
      }

      // Otherwise, let d be the current iteration.
      auto d = *currentIteration;
      // If playback direction is alternate-reverse increment d by 1.
      if (Direction == PlaybackDirection::AlternateReverse)
      {
        d++;
      }
      // If d % 2 == 0, let the current direction be forwards, otherwise let the current direction be reverse.
      // If d is infinity, let the current direction be forwards.
      if (std::isinf(d) || !std::fmod(d, 2))
      {
        return ComputedDirection::Forwards;
      }
      return ComputedDirection::Reverse;
    }();

    auto directedProgress = [&simpleIterationProgress, currentDirection]() -> Maybe<double>
    {
      // 3.9.1. Calculating the directed progress
      // https://drafts.csswg.org/web-animations-1/#calculating-the-directed-progress

      // The directed progress is calculated from the simple iteration progress using the following steps:

      // 1. If the simple iteration progress is unresolved, return unresolved.
      if (!simpleIterationProgress)
      {
        return null;
      }

      // 2. Calculate the current direction (we implement this as a separate method).

      // 3. If the current direction is forwards then return the simple iteration progress.
      if (currentDirection == ComputedDirection::Forwards)
      {
        return *simpleIterationProgress;
      }

      // Otherwise, return 1.0 - simple iteration progress.
      return 1 - *simpleIterationProgress;
    }();

    auto [transformedProgress, before] = [this, &directedProgress, currentDirection,
                                          phase]() -> Pair<Maybe<double>, TimingFunction::Before>
    {
      // 3.10.1. Calculating the transformed progress
      // https://drafts.csswg.org/web-animations-1/#calculating-the-transformed-progress
      auto before = TimingFunction::Before::No;

      // The transformed progress is calculated from the directed progress using the following steps:
      //
      // 1. If the directed progress is unresolved, return unresolved.
      if (!directedProgress)
      {
        return {null, before};
      }

      if (!IterationDuration.IsZero())
      {
        // 2. Calculate the value of the before flag as follows:
        // 1. Determine the current direction using the procedure defined in §3.9.1 Calculating the directed
        //    progress.
        // 2. If the current direction is forwards, let going forwards be true, otherwise it is false.
        bool goingForwards = currentDirection == ComputedDirection::Forwards;
        // 3. The before flag is set if the animation effect is in the before phase and going forwards is
        // true;
        //    or if the animation effect is in the after phase and going forwards is false.
        if ((phase == AnimationEffectPhase::Before && goingForwards)
            || (phase == AnimationEffectPhase::After && !goingForwards))
        {
          before = TimingFunction::Before::Yes;
        }

        // 3. Return the result of evaluating the animation effect’s timing function passing directed progress
        // as
        //    the input progress value and before flag as the before flag.
        auto transformProgressDuration = [&]()
        {
          if (auto time = IterationDuration.Time())
          {
            return time->count();
          }
          return 1.0;
        };

        return {TimingFunction->TransformProgress(*directedProgress, transformProgressDuration(), before),
                before};
      }

      return {*directedProgress, before};
    }();

    return {currentIteration, phase, transformedProgress, simpleIterationProgress, before};
  }
}

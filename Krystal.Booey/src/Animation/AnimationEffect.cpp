#include "Krystal.Booey/Animation/AnimationEffect.hpp"
#include "Krystal.Booey/Animation/AnimationTimeline.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Animation/WebAnimationUtilities.hpp"
#include <cmath>

namespace krys::boo
{
  AnimationEffect::AnimationEffect() noexcept = default;

  AnimationEffect::~AnimationEffect() noexcept = default;

  void AnimationEffect::SetAnimation(WebAnimation *animation) noexcept
  {
    if (_animation.get() == animation)
    {
      return;
    }

    _animation = CreateWeakPtr(animation);
    _timingDidMutate = true;
  }

  AnimationEffectTiming::ResolutionData AnimationEffect::GetResolutionData(
    UseCachedCurrentTime useCachedCurrentTime,
    EndpointInclusiveActiveInterval endpointInclusiveActiveInterval) const noexcept
  {
    if (!_animation)
    {
      return {};
    }

    RefPtr animation = ShareRefPtr(_animation.get());
    RefPtr timeline = ShareRefPtr(animation->Timeline());
    return {
      .TimelineTime = timeline ? timeline->CurrentTime(useCachedCurrentTime) : null,
      .TimelineDuration = timeline ? timeline->Duration() : null,
      .StartTime = animation->StartTime(),
      .LocalTime = animation->CurrentTime(useCachedCurrentTime),
      .EndpointInclusiveActiveInterval = endpointInclusiveActiveInterval,
      .PlaybackRate = animation->PlaybackRate(),
    };
  }

  BasicEffectTiming AnimationEffect::GetBasicTiming() noexcept
  {
    UpdateComputedTimingPropertiesIfNeeded();
    return _timing.GetBasicTiming(GetResolutionData());
  }

  ComputedEffectTiming AnimationEffect::GetComputedTiming(
    UseCachedCurrentTime useCachedCurrentTime,
    EndpointInclusiveActiveInterval endpointInclusiveActiveInterval) noexcept
  {
    UpdateComputedTimingPropertiesIfNeeded();
    auto data = GetResolutionData(useCachedCurrentTime, endpointInclusiveActiveInterval);
    auto resolvedTiming = _timing.Resolve(data);

    // https://drafts.csswg.org/web-animations-2/#dom-animationeffect-getcomputedtiming
    // The description of the duration attribute of the object needs to indicate that if timing.duration
    // is the string auto, this attribute will return the current calculated value of the intrinsic iteration
    // duration, which may be a expressed as a double representing the duration in milliseconds or a
    // percentage when the effect is associated with a progress-based timeline.
    const auto &computedDuration =
      _timing.SpecifiedIterationDuration ? _timing.IterationDuration : _timing.IntrinsicIterationDuration;

    ComputedEffectTiming computedTiming;
    computedTiming.Delay = SecondsToWebAnimationsAPITime(_timing.SpecifiedStartDelay);
    computedTiming.EndDelay = SecondsToWebAnimationsAPITime(_timing.SpecifiedEndDelay);
    computedTiming.Fill = _timing.Fill == FillMode::Auto ? FillMode::None : _timing.Fill;
    computedTiming.IterationStart = _timing.IterationStart;
    computedTiming.Iterations = _timing.Iterations;
    computedTiming.Duration = computedDuration;
    computedTiming.Direction = _timing.Direction;
    computedTiming.EndTime = _timing.EndTime;
    computedTiming.ActiveDuration = _timing.ActiveDuration;
    computedTiming.LocalTime = data.LocalTime;
    computedTiming.SimpleIterationProgress = resolvedTiming.SimpleIterationProgress;
    computedTiming.Progress = resolvedTiming.TransformedProgress;
    computedTiming.CurrentIteration = resolvedTiming.CurrentIteration;
    computedTiming.Phase = resolvedTiming.Phase;
    computedTiming.Before = resolvedTiming.Before;
    return computedTiming;
  }

  bool AnimationEffect::SetIterationStart(double iterationStart) noexcept
  {
    // https://drafts.csswg.org/web-animations-1/#dom-animationeffecttiming-iterationstart
    // If an attempt is made to set this attribute to a value less than zero, a TypeError must
    // be thrown and the value of the iterationStart attribute left unchanged.
    if (iterationStart < 0)
    {
      return false;
    }

    _timing.IterationStart = iterationStart;
    return true;
  }

  bool AnimationEffect::SetIterations(double iterations) noexcept
  {
    // https://drafts.csswg.org/web-animations-1/#dom-animationeffecttiming-iterations
    // If an attempt is made to set this attribute to a value less than zero or a NaN value, a
    // TypeError must be thrown and the value of the iterations attribute left unchanged.
    if (iterations < 0 || std::isnan(iterations))
    {
      return false;
    }

    if (_timing.Iterations == iterations)
    {
      return true;
    }

    _timing.Iterations = iterations;
    _timingDidMutate = true;
    return true;
  }

  WebAnimationTime AnimationEffect::Delay() noexcept
  {
    UpdateComputedTimingPropertiesIfNeeded();
    return _timing.StartDelay;
  }

  void AnimationEffect::SetDelay(const Seconds &delay) noexcept
  {
    if (_timing.SpecifiedStartDelay == delay)
    {
      return;
    }

    _timing.SpecifiedStartDelay = delay;
    _timingDidMutate = true;
  }

  WebAnimationTime AnimationEffect::EndDelay() noexcept
  {
    UpdateComputedTimingPropertiesIfNeeded();
    return _timing.EndDelay;
  }

  void AnimationEffect::SetEndDelay(const Seconds &endDelay) noexcept
  {
    if (_timing.SpecifiedEndDelay == endDelay)
    {
      return;
    }

    _timing.SpecifiedEndDelay = endDelay;
    _timingDidMutate = true;
  }

  void AnimationEffect::SetFill(FillMode fill) noexcept
  {
    _timing.Fill = fill;
  }

  WebAnimationTime AnimationEffect::IterationDuration() noexcept
  {
    UpdateComputedTimingPropertiesIfNeeded();
    return _timing.IterationDuration;
  }

  WebAnimationTime AnimationEffect::IterationDuration() const noexcept
  {
    krys_debug_assert(!_timingDidMutate);
    return _timing.IterationDuration;
  }

  void AnimationEffect::SetIterationDuration(const Maybe<Seconds> &duration) noexcept
  {
    if (_timing.SpecifiedIterationDuration == duration)
    {
      return;
    }

    _timing.SpecifiedIterationDuration = duration;
    _timingDidMutate = true;
  }

  void AnimationEffect::SetDirection(PlaybackDirection direction) noexcept
  {
    _timing.Direction = direction;
  }

  void AnimationEffect::SetTimingFunction(const RefPtr<boo::TimingFunction> &timingFunction) noexcept
  {
    _timing.TimingFunction = timingFunction;
  }

  WebAnimationTime AnimationEffect::ActiveDuration() noexcept
  {
    UpdateComputedTimingPropertiesIfNeeded();
    return _timing.ActiveDuration;
  }

  WebAnimationTime AnimationEffect::EndTime() noexcept
  {
    UpdateComputedTimingPropertiesIfNeeded();
    return _timing.EndTime;
  }

  Maybe<double> AnimationEffect::ProgressUntilNextStep(double iterationProgress) const noexcept
  {
    auto *stepsTimingFunction = DynamicDowncast<StepsTimingFunction>(_timing.TimingFunction.get());
    if (!stepsTimingFunction)
    {
      return null;
    }

    auto numberOfSteps = stepsTimingFunction->NumberOfSteps();
    auto nextStepProgress = std::ceil(iterationProgress * numberOfSteps) / numberOfSteps;
    return nextStepProgress - iterationProgress;
  }

  Seconds AnimationEffect::TimeToNextTick(const BasicEffectTiming &timing) noexcept
  {
    constexpr auto infinity = Seconds {std::numeric_limits<double>::infinity()};

    switch (timing.Phase)
    {
      case AnimationEffectPhase::Before:
        // The effect is in its "before" phase, in this case we can wait until it enters its "active" phase.
        return static_cast<Seconds>(Delay() - *timing.LocalTime);
      case AnimationEffectPhase::Active:
      {
        if (!TicksContinuouslyWhileActive())
        {
          return static_cast<Seconds>(EndTime() - *timing.LocalTime);
        }
        if (auto iterationProgress = GetComputedTiming().SimpleIterationProgress)
        {
          // In case we're in a range that uses a steps() timing function, we can compute the time until the
          // next step starts.
          if (auto progressUntilNextStep = ProgressUntilNextStep(*iterationProgress))
          {
            return static_cast<Seconds>(IterationDuration() * *progressUntilNextStep);
          }
        }
        // Other effects that continuously tick in the "active" phase will need to update their animated
        // progress at the immediate next opportunity.
        return Seconds {0};
      }
      case AnimationEffectPhase::After:
        // The effect is in its after phase, which means it will no longer update its progress, so it doens't
        // need a tick.
        return infinity;
      case AnimationEffectPhase::Idle: krys_debug_assert(false); return infinity;
    }

    krys_debug_assert(false);
    return infinity;
  }

  void AnimationEffect::AnimationTimelineDidChange() noexcept
  {
    _timingDidMutate = true;
  }

  void AnimationEffect::AnimationPlaybackRateDidChange() noexcept
  {
    _timingDidMutate = true;
  }

  void AnimationEffect::UpdateComputedTimingPropertiesIfNeeded() noexcept
  {
    if (!_timingDidMutate)
    {
      return;
    }

    _timingDidMutate = false;

    auto playbackRate = _animation ? _animation->PlaybackRate() : 1.0;

    // FIXME: The duration of the animation attachment range for scroll timelines, once there are scroll
    // timelines.
    auto rangeDuration = [&]() -> Maybe<WebAnimationTime>
    {
      RefPtr animation = ShareRefPtr(_animation.get());
      if (!animation)
      {
        return null;
      }
      RefPtr timeline = ShareRefPtr(animation->Timeline());
      if (!timeline)
      {
        return null;
      }
      return timeline->Duration();
    }();

    _timing.UpdateComputedProperties(rangeDuration, playbackRate);
  }
}

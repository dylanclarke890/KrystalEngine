#include "Krystal.Booey/Animation/AnimationEffect.hpp"
#include "Krystal.Booey/Animation/AnimationTimeline.hpp"
#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Animation/WebAnimationUtilities.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Easing.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericFactory.hpp"
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

  EffectTiming AnimationEffect::GetBindingsTiming() const noexcept
  {
    if (RefPtr styleOriginatedAnimation = ShareRefPtr(DynamicDowncast<StyleOriginatedAnimation>(Animation())))
    {
      styleOriginatedAnimation->FlushPendingStyleChanges();
    }

    EffectTiming timing;
    timing.Delay = SecondsToWebAnimationsAPITime(_timing.SpecifiedStartDelay);
    timing.EndDelay = SecondsToWebAnimationsAPITime(_timing.SpecifiedEndDelay);
    timing.Fill = _timing.Fill;
    timing.IterationStart = _timing.IterationStart;
    timing.Iterations = _timing.Iterations;
    if (auto specifiedDuration = _timing.SpecifiedIterationDuration)
    {
      timing.Duration = SecondsToWebAnimationsAPITime(*specifiedDuration);
    }
    else
    {
      timing.Duration = dom::DOMString(u8"auto");
    }
    timing.Direction = _timing.Direction;
    timing.Easing = _timing.TimingFunction->CssText();
    return timing;
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

  ComputedEffectTiming AnimationEffect::GetBindingsComputedTiming() noexcept
  {
    if (RefPtr styleOriginatedAnimation = ShareRefPtr(DynamicDowncast<StyleOriginatedAnimation>(Animation())))
    {
      styleOriginatedAnimation->FlushPendingStyleChanges();
    }
    return GetComputedTiming();
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
    auto computedDuration = [&]() -> DoubleOrCSSNumericValueOrString
    {
      const auto &duration =
        _timing.SpecifiedIterationDuration ? _timing.IterationDuration : _timing.IntrinsicIterationDuration;
      if (auto percent = duration.Percentage())
      {
        return RefPtr<css::CSSNumericValue> {css::CSSNumericFactory::Percent(*percent)};
      }
      krys_debug_assert(duration.Time());
      return SecondsToWebAnimationsAPITime(*duration.Time());
    }();

    ComputedEffectTiming computedTiming;
    computedTiming.Delay = SecondsToWebAnimationsAPITime(_timing.SpecifiedStartDelay);
    computedTiming.EndDelay = SecondsToWebAnimationsAPITime(_timing.SpecifiedEndDelay);
    computedTiming.Fill = _timing.Fill == FillMode::Auto ? FillMode::None : _timing.Fill;
    computedTiming.IterationStart = _timing.IterationStart;
    computedTiming.Iterations = _timing.Iterations;
    computedTiming.Duration = computedDuration;
    computedTiming.Direction = _timing.Direction;
    computedTiming.Easing = _timing.TimingFunction->CssText();
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

  dom::ExceptionOr<void>
    AnimationEffect::BindingsUpdateTiming(dom::Document &document,
                                          const Maybe<OptionalEffectTiming> &timing) noexcept
  {
    auto retVal = UpdateTiming(document, timing);
    if (!retVal.HasException() && timing)
    {
      if (RefPtr cssAnimation = ShareRefPtr(DynamicDowncast<CSSAnimation>(Animation())))
      {
        cssAnimation->EffectTimingWasUpdatedUsingBindings(*timing);
      }
    }
    return retVal;
  }

  dom::ExceptionOr<void> AnimationEffect::UpdateTiming(dom::Document &document,
                                                       const Maybe<OptionalEffectTiming> &timing) noexcept
  {
    // 6.5.4. Updating the timing of an AnimationEffect
    // https://drafts.csswg.org/web-animations/#updating-animationeffect-timing

    // To update the timing properties of an animation effect, effect, from an EffectTiming or
    // OptionalEffectTiming object, input, perform the following steps:
    if (!timing)
    {
      return {};
    }

    // 1. If the iterationStart member of input is present and less than zero, throw a TypeError and abort
    // this procedure.
    if (timing->IterationStart)
    {
      if (*timing->IterationStart < 0)
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
    }

    // 2. If the iterations member of input is present, and less than zero or is the value NaN, throw a
    // TypeError and abort this procedure.
    if (timing->Iterations)
    {
      if (*timing->Iterations < 0 || std::isnan(*timing->Iterations))
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
    }

    // 3. If the duration member of input is present, and less than zero or is the value NaN, throw a
    // TypeError and abort this procedure.
    // FIXME: should it not throw an exception on a string other than "auto"?
    if (timing->Duration)
    {
      if (const auto *durationAsDouble = std::get_if<double>(&*timing->Duration))
      {
        if (*durationAsDouble < 0 || std::isnan(*durationAsDouble))
        {
          return dom::Exception {dom::ExceptionCode::TypeError};
        }
      }
      else if (std::get<dom::DOMString>(*timing->Duration) != u8"auto")
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
    }

    if (auto iterations = timing->Iterations)
    {
      // https://github.com/w3c/csswg-drafts/issues/11343
      if (std::isinf(*iterations))
      {
        if (RefPtr animation = ShareRefPtr(_animation.get()))
        {
          if (RefPtr timeline = ShareRefPtr(animation->Timeline()))
          {
            if (timeline->IsProgressBased())
            {
              return dom::Exception {
                dom::ExceptionCode::TypeError,
                u8"The number of iterations cannot be set to Infinity for progress-based animations"};
            }
          }
        }
      }
    }

    // 4. If the easing member of input is present but cannot be parsed using the <timing-function> production
    // [CSS-EASING-1], throw a TypeError and abort this procedure.
    if (timing->Easing)
    {
      css::ParserContext parsingContext(document);
      // FIXME: Determine the how calc() and relative units should be resolved and switch to the
      // non-deprecated parsing function.
      auto timingFunctionResult =
        css::PropertyParserHelpers::ParseEasingFunctionDeprecated(*timing->Easing, parsingContext);
      if (!timingFunctionResult)
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      SetTimingFunction(timingFunctionResult);
    }

    // 5. Assign each member present in input to the corresponding timing property of effect as follows:
    //
    //    delay → start delay
    //    endDelay → end delay
    //    fill → fill mode
    //    iterationStart → iteration start
    //    iterations → iteration count
    //    duration → iteration duration
    //    direction → playback direction
    //    easing → timing function

    if (auto delay = timing->Delay)
    {
      SetDelay(Seconds {*delay / 1'000});
    }

    if (auto endDelay = timing->EndDelay)
    {
      SetEndDelay(Seconds {*endDelay / 1'000});
    }

    if (auto fill = timing->Fill)
    {
      SetFill(*fill);
    }

    if (auto iterationStart = timing->IterationStart)
    {
      (void)SetIterationStart(*iterationStart);
    }

    if (auto iterations = timing->Iterations)
    {
      (void)SetIterations(*iterations);
    }

    if (const auto &duration = timing->Duration)
    {
      if (const auto *durationDouble = std::get_if<double>(&*duration))
      {
        SetIterationDuration(Seconds {*durationDouble / 1'000});
      }
      else
      {
        SetIterationDuration(null);
      }
    }

    if (auto direction = timing->Direction)
    {
      SetDirection(*direction);
    }

    if (RefPtr animation = ShareRefPtr(_animation.get()))
    {
      animation->EffectTimingDidChange();
    }

    return {};
  }

  dom::ExceptionOr<void> AnimationEffect::SetIterationStart(double iterationStart) noexcept
  {
    // https://drafts.csswg.org/web-animations-1/#dom-animationeffecttiming-iterationstart
    // If an attempt is made to set this attribute to a value less than zero, a TypeError must
    // be thrown and the value of the iterationStart attribute left unchanged.
    if (iterationStart < 0)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    if (_timing.IterationStart == iterationStart)
    {
      return {};
    }

    _timing.IterationStart = iterationStart;

    return {};
  }

  dom::ExceptionOr<void> AnimationEffect::SetIterations(double iterations) noexcept
  {
    // https://drafts.csswg.org/web-animations-1/#dom-animationeffecttiming-iterations
    // If an attempt is made to set this attribute to a value less than zero or a NaN value, a
    // TypeError must be thrown and the value of the iterations attribute left unchanged.
    if (iterations < 0 || std::isnan(iterations))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    if (_timing.Iterations == iterations)
    {
      return {};
    }

    _timing.Iterations = iterations;
    _timingDidMutate = true;

    return {};
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

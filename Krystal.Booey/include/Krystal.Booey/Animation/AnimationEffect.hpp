#pragma once

#include "Krystal.Booey/Animation/AnimationEffectTiming.hpp"
#include "Krystal.Booey/Animation/ComputedEffectTiming.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class WebAnimation;

  /// @brief Something an animation plays: its timing, and what it does at each point in its timing.
  /// @see https://drafts.csswg.org/web-animations-1/#animation-effects
  // FIXME: The Web Animations interface (`getTiming()`, `getComputedTiming()` and `updateTiming()` with their
  // script types), once there's scripting.
  class AnimationEffect : public RefCounted<AnimationEffect>, public CanMakeWeakPtr<AnimationEffect>
  {
  private:
    AnimationEffectTiming _timing;
    WeakPtr<WebAnimation> _animation;
    bool _timingDidMutate {false};

  protected:
    AnimationEffect() noexcept;

  public:
    virtual ~AnimationEffect() noexcept;

    KRYS_NODISCARD virtual bool IsKeyframeEffect() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD BasicEffectTiming GetBasicTiming() noexcept;
    KRYS_NODISCARD ComputedEffectTiming
      GetComputedTiming(UseCachedCurrentTime useCachedCurrentTime = UseCachedCurrentTime::Yes,
                        EndpointInclusiveActiveInterval endpointInclusiveActiveInterval =
                          EndpointInclusiveActiveInterval::No) noexcept;

    virtual void AnimationDidTick() noexcept
    {
    }
    virtual void AnimationBecameReady() noexcept
    {
    }
    virtual void AnimationDidChangeTimingProperties() noexcept
    {
    }
    virtual void AnimationWasCanceled() noexcept
    {
    }
    virtual void AnimationSuspensionStateDidChange(bool) noexcept
    {
    }
    virtual void AnimationTimelineDidChange() noexcept;
    virtual void AnimationDidFinish() noexcept
    {
    }
    virtual void AnimationPlaybackRateDidChange() noexcept;

    KRYS_NODISCARD const AnimationEffectTiming &Timing() const noexcept
    {
      return _timing;
    }

    KRYS_NODISCARD WebAnimation *Animation() const noexcept
    {
      return _animation.get();
    }
    virtual void SetAnimation(WebAnimation *animation) noexcept;

    KRYS_NODISCARD WebAnimationTime Delay() noexcept;
    KRYS_NODISCARD Seconds SpecifiedDelay() const noexcept
    {
      return _timing.SpecifiedStartDelay;
    }
    void SetDelay(const Seconds &delay) noexcept;

    KRYS_NODISCARD WebAnimationTime EndDelay() noexcept;
    KRYS_NODISCARD Seconds SpecifiedEndDelay() const noexcept
    {
      return _timing.SpecifiedEndDelay;
    }
    void SetEndDelay(const Seconds &endDelay) noexcept;

    KRYS_NODISCARD FillMode Fill() const noexcept
    {
      return _timing.Fill;
    }
    void SetFill(FillMode fill) noexcept;

    KRYS_NODISCARD double IterationStart() const noexcept
    {
      return _timing.IterationStart;
    }
    /// @return False, leaving the iteration start unchanged, if it's negative.
    bool SetIterationStart(double iterationStart) noexcept;

    KRYS_NODISCARD double Iterations() const noexcept
    {
      return _timing.Iterations;
    }
    /// @return False, leaving the iteration count unchanged, if it's negative or NaN.
    bool SetIterations(double iterations) noexcept;

    KRYS_NODISCARD WebAnimationTime IterationDuration() noexcept;
    /// @attention The computed timing properties must be up to date.
    KRYS_NODISCARD WebAnimationTime IterationDuration() const noexcept;
    KRYS_NODISCARD Maybe<Seconds> SpecifiedIterationDuration() const noexcept
    {
      return _timing.SpecifiedIterationDuration;
    }
    void SetIterationDuration(const Maybe<Seconds> &duration) noexcept;

    KRYS_NODISCARD PlaybackDirection Direction() const noexcept
    {
      return _timing.Direction;
    }
    void SetDirection(PlaybackDirection direction) noexcept;

    KRYS_NODISCARD boo::TimingFunction *TimingFunction() const noexcept
    {
      return _timing.TimingFunction.get();
    }
    void SetTimingFunction(const RefPtr<boo::TimingFunction> &timingFunction) noexcept;

    KRYS_NODISCARD WebAnimationTime ActiveDuration() noexcept;
    KRYS_NODISCARD WebAnimationTime EndTime() noexcept;

    /// @brief How long until the effect next needs updating, given its timing.
    KRYS_NODISCARD virtual Seconds TimeToNextTick(const BasicEffectTiming &timing) noexcept;

    KRYS_NODISCARD virtual bool PreventsAnimationReadiness() const noexcept
    {
      return false;
    }

  protected:
    KRYS_NODISCARD virtual bool TicksContinuouslyWhileActive() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual Maybe<double> ProgressUntilNextStep(double iterationProgress) const noexcept;

  private:
    KRYS_NODISCARD AnimationEffectTiming::ResolutionData
      GetResolutionData(UseCachedCurrentTime useCachedCurrentTime = UseCachedCurrentTime::Yes,
                        EndpointInclusiveActiveInterval endpointInclusiveActiveInterval =
                          EndpointInclusiveActiveInterval::No) const noexcept;
    void UpdateComputedTimingPropertiesIfNeeded() noexcept;
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_ANIMATION_EFFECT(ToValueTypeName, predicate)                             \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(ToValueTypeName)                                                    \
    KRYS_NODISCARD static bool IsType(const krys::boo::AnimationEffect &value) noexcept                      \
    {                                                                                                        \
      return value.predicate;                                                                                \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

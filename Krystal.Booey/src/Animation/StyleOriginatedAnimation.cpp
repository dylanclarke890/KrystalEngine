#include "Krystal.Booey/Animation/StyleOriginatedAnimation.hpp"
#include "Krystal.Booey/Animation/AnimationEffect.hpp"
#include "Krystal.Booey/Animation/DocumentTimeline.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/StyleOriginatedAnimationEvent.hpp"
#include "Krystal.Booey/Animation/WebAnimationUtilities.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"

namespace krys::boo
{
  StyleOriginatedAnimation::StyleOriginatedAnimation(const style::Styleable &styleable) noexcept
      : WebAnimation(styleable.Element.NodeDocument()), _owningElement(CreateWeakPtr(&styleable.Element)),
        _owningPseudoElementIdentifier(styleable.PseudoElementIdentifier)
  {
  }

  StyleOriginatedAnimation::~StyleOriginatedAnimation() noexcept = default;

  Maybe<style::Styleable> StyleOriginatedAnimation::OwningElement() const noexcept
  {
    if (_owningElement)
    {
      return style::Styleable(*_owningElement, _owningPseudoElementIdentifier);
    }
    return null;
  }

  void StyleOriginatedAnimation::Tick() noexcept
  {
    bool wasRelevant = IsRelevant();

    WebAnimation::Tick();
    InvalidateDOMEvents();

    // If a style-originated animation transitions from a non-idle state to an idle state, it means it was
    // canceled using the Web Animations API and it should be disassociated from its owner element. From this
    // point on, this animation is like any other animation and should not appear in the maps containing
    // running CSS Transitions and CSS Animations for a given element.
    if (wasRelevant && GetPlayState() == WebAnimation::PlayState::Idle)
    {
      DisassociateFromOwningElement();
    }
  }

  bool StyleOriginatedAnimation::CanHaveGlobalPosition() noexcept
  {
    // https://drafts.csswg.org/css-animations-2/#animation-composite-order
    // https://drafts.csswg.org/css-transitions-2/#animation-composite-order
    // CSS Animations and CSS Transitions generated using the markup defined in this specification are not
    // added to the global animation list when they are created. Instead, these animations are appended to the
    // global animation list at the first moment when they transition out of the idle play state after being
    // disassociated from their owning element.
    return !_owningElement && GetPlayState() != WebAnimation::PlayState::Idle;
  }

  void StyleOriginatedAnimation::DisassociateFromOwningElement() noexcept
  {
    if (!_owningElement)
    {
      return;
    }

    OwningElement()->RemoveStyleOriginatedAnimationFromListsForOwningElement(*this);
    _owningElement = nullptr;
  }

  void StyleOriginatedAnimation::Initialize(const RenderStyle *oldStyle, const RenderStyle &newStyle,
                                            const style::ResolutionContext &resolutionContext) noexcept
  {
    WebAnimation::Initialize();

    // We need to suspend invalidation of the animation's keyframe effect during its creation as it would
    // otherwise trigger invalidation of the document's style and this would be incorrect since it would
    // happen during style invalidation.
    SuspendEffectInvalidation();

    krys_debug_assert(_owningElement);

    Ref effect = KeyframeEffect::Create(*_owningElement, _owningPseudoElementIdentifier);
    SetEffect(effect);
    SetTimeline(ShareRefPtr<AnimationTimeline>(&_owningElement->NodeDocument().Timeline()));
    effect->ComputeStyleOriginatedAnimationBlendingKeyframes(oldStyle, newStyle, resolutionContext);
    SyncPropertiesWithBackingAnimation();
    if (BackingAnimationPlayState() == AnimationPlayState::Running)
    {
      (void)Play();
    }
    else
    {
      (void)Pause();
    }

    UnsuspendEffectInvalidation();
  }

  void StyleOriginatedAnimation::SyncPropertiesWithBackingAnimation() noexcept
  {
  }

  Maybe<WebAnimationTime> StyleOriginatedAnimation::BindingsStartTime() const noexcept
  {
    FlushPendingStyleChanges();
    return WebAnimation::BindingsStartTime();
  }

  Maybe<WebAnimationTime> StyleOriginatedAnimation::BindingsCurrentTime() const noexcept
  {
    FlushPendingStyleChanges();
    return WebAnimation::BindingsCurrentTime();
  }

  WebAnimation::PlayState StyleOriginatedAnimation::BindingsPlayState() const noexcept
  {
    FlushPendingStyleChanges();
    return WebAnimation::BindingsPlayState();
  }

  WebAnimation::ReplaceState StyleOriginatedAnimation::BindingsReplaceState() const noexcept
  {
    FlushPendingStyleChanges();
    return WebAnimation::BindingsReplaceState();
  }

  bool StyleOriginatedAnimation::BindingsPending() const noexcept
  {
    FlushPendingStyleChanges();
    return WebAnimation::BindingsPending();
  }

  AnimationPromise &StyleOriginatedAnimation::BindingsReady() noexcept
  {
    FlushPendingStyleChanges();
    return WebAnimation::BindingsReady();
  }

  AnimationPromise &StyleOriginatedAnimation::BindingsFinished() noexcept
  {
    FlushPendingStyleChanges();
    return WebAnimation::BindingsFinished();
  }

  dom::ExceptionOr<void> StyleOriginatedAnimation::BindingsPlay() noexcept
  {
    FlushPendingStyleChanges();
    return WebAnimation::BindingsPlay();
  }

  dom::ExceptionOr<void> StyleOriginatedAnimation::BindingsPause() noexcept
  {
    FlushPendingStyleChanges();
    return WebAnimation::BindingsPause();
  }

  void StyleOriginatedAnimation::FlushPendingStyleChanges() const noexcept
  {
    if (RefPtr keyframeEffect = ShareRefPtr(GetKeyframeEffect()))
    {
      if (RefPtr target = ShareRefPtr(keyframeEffect->Target()))
      {
        (void)target->NodeDocument().UpdateStyleIfNeeded();
      }
    }
  }

  void StyleOriginatedAnimation::SetTimeline(RefPtr<AnimationTimeline> &&newTimeline) noexcept
  {
    if (Timeline() && !newTimeline)
    {
      auto cancelationTime = ComputeCancelationTime();
      Ref protectedThis = ShareRef(*this);
      WebAnimation::SetTimeline(nullptr);
      InvalidateDOMEvents(cancelationTime);
    }
    else
    {
      WebAnimation::SetTimeline(krys::move(newTimeline));
    }
  }

  void StyleOriginatedAnimation::Cancel(WebAnimation::Silently silently) noexcept
  {
    auto cancelationTime = ComputeCancelationTime();

    Ref protectedThis = ShareRef(*this);
    WebAnimation::Cancel(silently);

    if (!_owningElement)
    {
      return;
    }

    auto isPending = Pending();
    if (isPending && _wasPending)
    {
      return;
    }

    bool wasIdle = _previousPhase == AnimationEffectPhase::Idle;
    bool wasAfter = _previousPhase == AnimationEffectPhase::After;

    if (!wasIdle && !wasAfter)
    {
      if (IsCSSAnimation())
      {
        EnqueueDOMEvent(EventNames::AnimationCancel, cancelationTime, cancelationTime);
      }
      else if (IsCSSTransition())
      {
        EnqueueDOMEvent(EventNames::TransitionCancel, cancelationTime, cancelationTime);
      }
    }

    _wasPending = isPending;
    _previousPhase = AnimationEffectPhase::Idle;
    _previousIteration = 0;
  }

  void StyleOriginatedAnimation::CancelFromStyle(WebAnimation::Silently silently) noexcept
  {
    Cancel(silently);
    DisassociateFromOwningElement();
  }

  AnimationEffectPhase StyleOriginatedAnimation::PhaseWithoutEffect() const noexcept
  {
    // This shouldn't be called if we actually have an effect.
    krys_debug_assert(!Effect());

    auto animationCurrentTime = CurrentTime();
    if (!animationCurrentTime)
    {
      return AnimationEffectPhase::Idle;
    }

    // Since we don't have an effect, the duration will be zero so the phase is 'before' if the current time
    // is less than zero.
    return *animationCurrentTime < animationCurrentTime->MatchingZero() ? AnimationEffectPhase::Before
                                                                        : AnimationEffectPhase::After;
  }

  WebAnimationTime StyleOriginatedAnimation::EffectTimeAtStart() const noexcept
  {
    if (RefPtr effect = ShareRefPtr(Effect()))
    {
      return effect->Delay();
    }
    return Seconds {0};
  }

  WebAnimationTime StyleOriginatedAnimation::EffectTimeAtIteration(double iteration) const noexcept
  {
    if (RefPtr effect = ShareRefPtr(Effect()))
    {
      auto iterationDuration = effect->IterationDuration();
      // We need not account for delay with progress-based animations as the Web Animations spec does not
      // specify how to account for them.
      if (iterationDuration.Percentage())
      {
        return iterationDuration * iteration;
      }
      return effect->Delay() + iterationDuration * iteration;
    }
    return Seconds {0};
  }

  WebAnimationTime StyleOriginatedAnimation::EffectTimeAtEnd() const noexcept
  {
    if (RefPtr effect = ShareRefPtr(Effect()))
    {
      return effect->EndTime();
    }
    return Seconds {0};
  }

  WebAnimationTime StyleOriginatedAnimation::ComputeCancelationTime() const noexcept
  {
    if (_owningElement)
    {
      if (RefPtr animationEffect = ShareRefPtr(Effect()))
      {
        if (auto activeTime = animationEffect->GetBasicTiming().ActiveTime)
        {
          return *activeTime;
        }
      }
    }
    return Seconds {0};
  }

  void StyleOriginatedAnimation::InvalidateDOMEvents(WebAnimationTime cancelationTime) noexcept
  {
    if (!_owningElement)
    {
      return;
    }

    auto isPending = Pending();
    if (isPending && _wasPending)
    {
      return;
    }

    double iteration = 0;
    AnimationEffectPhase currentPhase;
    WebAnimationTime intervalStart;
    WebAnimationTime intervalEnd;

    RefPtr animationEffect = ShareRefPtr(Effect());
    if (animationEffect)
    {
      auto timing = animationEffect->GetComputedTiming();
      if (auto computedIteration = timing.CurrentIteration)
      {
        iteration = *computedIteration;
      }
      currentPhase = timing.Phase;
      if (timing.ActiveDuration.Percentage())
      {
        // We need not account for delay with progress-based animations as the Web Animations spec does not
        // specify how to account for them.
        auto zero = timing.ActiveDuration.MatchingZero();
        intervalStart = std::max(zero, timing.ActiveDuration);
        intervalEnd = std::max(zero, std::min(timing.EndTime, timing.ActiveDuration));
      }
      else
      {
        auto activeDuration = timing.ActiveDuration.Time()->count() * 1'000;
        intervalStart = std::max(Seconds {0}, Seconds {std::min(-timing.Delay, activeDuration) / 1'000});
        intervalEnd = std::max(
          Seconds {0},
          Seconds {std::min(timing.EndTime.Time()->count() * 1'000 - timing.Delay, activeDuration) / 1'000});
      }
    }
    else
    {
      iteration = 0;
      currentPhase = PhaseWithoutEffect();
      intervalStart = Seconds {0};
      intervalEnd = Seconds {0};
    }

    bool wasActive = _previousPhase == AnimationEffectPhase::Active;
    bool wasAfter = _previousPhase == AnimationEffectPhase::After;
    bool wasBefore = _previousPhase == AnimationEffectPhase::Before;
    bool wasIdle = _previousPhase == AnimationEffectPhase::Idle;

    bool isActive = currentPhase == AnimationEffectPhase::Active;
    bool isAfter = currentPhase == AnimationEffectPhase::After;
    bool isBefore = currentPhase == AnimationEffectPhase::Before;
    bool isIdle = currentPhase == AnimationEffectPhase::Idle;

    if (IsCSSAnimation())
    {
      // https://drafts.csswg.org/css-animations-2/#events
      if ((wasIdle || wasBefore) && isActive)
      {
        EnqueueDOMEvent(EventNames::AnimationStart, intervalStart, EffectTimeAtStart());
      }
      else if ((wasIdle || wasBefore) && isAfter)
      {
        EnqueueDOMEvent(EventNames::AnimationStart, intervalStart, EffectTimeAtStart());
        EnqueueDOMEvent(EventNames::AnimationEnd, intervalEnd, EffectTimeAtEnd());
      }
      else if (wasActive && isBefore)
      {
        EnqueueDOMEvent(EventNames::AnimationEnd, intervalStart, EffectTimeAtEnd());
      }
      else if (wasActive && isActive && _previousIteration != iteration)
      {
        auto iterationBoundary = iteration;
        if (_previousIteration > iteration)
        {
          iterationBoundary++;
        }
        auto elapsedTime = animationEffect ? animationEffect->IterationDuration()
                                               * (iterationBoundary - animationEffect->IterationStart())
                                           : ZeroTime();
        EnqueueDOMEvent(EventNames::AnimationIteration, elapsedTime, EffectTimeAtIteration(iteration));
      }
      else if (wasActive && isAfter)
      {
        EnqueueDOMEvent(EventNames::AnimationEnd, intervalEnd, EffectTimeAtEnd());
      }
      else if (wasAfter && isActive)
      {
        EnqueueDOMEvent(EventNames::AnimationStart, intervalEnd, EffectTimeAtStart());
      }
      else if (wasAfter && isBefore)
      {
        EnqueueDOMEvent(EventNames::AnimationStart, intervalEnd, EffectTimeAtStart());
        EnqueueDOMEvent(EventNames::AnimationEnd, intervalStart, EffectTimeAtEnd());
      }
      else if ((!wasIdle && !wasAfter) && isIdle)
      {
        EnqueueDOMEvent(EventNames::AnimationCancel, cancelationTime, cancelationTime);
      }
    }
    else if (IsCSSTransition())
    {
      // https://drafts.csswg.org/css-transitions-2/#transition-events
      if (wasIdle && (isPending || isBefore))
      {
        EnqueueDOMEvent(EventNames::TransitionRun, intervalStart, EffectTimeAtStart());
      }
      else if (wasIdle && isActive)
      {
        auto scheduledEffectTime = EffectTimeAtStart();
        EnqueueDOMEvent(EventNames::TransitionRun, intervalStart, scheduledEffectTime);
        EnqueueDOMEvent(EventNames::TransitionStart, intervalStart, scheduledEffectTime);
      }
      else if (wasIdle && isAfter)
      {
        EnqueueDOMEvent(EventNames::TransitionRun, intervalStart, EffectTimeAtStart());
        EnqueueDOMEvent(EventNames::TransitionStart, intervalStart, EffectTimeAtStart());
        EnqueueDOMEvent(EventNames::TransitionEnd, intervalEnd, EffectTimeAtEnd());
      }
      else if ((_wasPending || wasBefore) && isActive)
      {
        EnqueueDOMEvent(EventNames::TransitionStart, intervalStart, EffectTimeAtStart());
      }
      else if ((_wasPending || wasBefore) && isAfter)
      {
        EnqueueDOMEvent(EventNames::TransitionStart, intervalStart, EffectTimeAtStart());
        EnqueueDOMEvent(EventNames::TransitionEnd, intervalEnd, EffectTimeAtEnd());
      }
      else if (wasActive && isAfter)
      {
        EnqueueDOMEvent(EventNames::TransitionEnd, intervalEnd, EffectTimeAtEnd());
      }
      else if (wasActive && isBefore)
      {
        EnqueueDOMEvent(EventNames::TransitionEnd, intervalStart, EffectTimeAtEnd());
      }
      else if (wasAfter && isActive)
      {
        EnqueueDOMEvent(EventNames::TransitionStart, intervalEnd, EffectTimeAtStart());
      }
      else if (wasAfter && isBefore)
      {
        EnqueueDOMEvent(EventNames::TransitionStart, intervalEnd, EffectTimeAtStart());
        EnqueueDOMEvent(EventNames::TransitionEnd, intervalStart, EffectTimeAtEnd());
      }
      else if ((!wasIdle && !wasAfter) && isIdle)
      {
        EnqueueDOMEvent(EventNames::TransitionCancel, cancelationTime, cancelationTime);
      }
    }

    _wasPending = isPending;
    _previousPhase = currentPhase;
    _previousIteration = iteration;
  }

  void StyleOriginatedAnimation::EnqueueDOMEvent(dom::DOMStringAtom eventType, WebAnimationTime elapsedTime,
                                                 WebAnimationTime scheduledEffectTime) noexcept
  {
    if (!_owningElement)
    {
      return;
    }

    auto scheduledTimelineTime = [&]() -> Maybe<Seconds>
    {
      if (auto *documentTimeline = DynamicDowncast<DocumentTimeline>(Timeline()))
      {
        krys_debug_assert(scheduledEffectTime.Time());
        if (auto scheduledAnimationTime = ConvertAnimationTimeToTimelineTime(*scheduledEffectTime.Time()))
        {
          return documentTimeline->ConvertTimelineTimeToOriginRelativeTime(*scheduledAnimationTime);
        }
      }
      return null;
    }();

    auto time = [&]()
    {
      if (auto seconds = elapsedTime.Time())
      {
        return SecondsToWebAnimationsAPITime(*seconds) / 1'000;
      }
      return 0.0;
    };

    auto event = CreateEvent(eventType, scheduledTimelineTime, time(), _owningPseudoElementIdentifier);
    event->SetTarget(ShareRefPtr<dom::EventTarget>(_owningElement.get()));
    EnqueueAnimationEvent(krys::move(event));
  }
}

#include "Krystal.Booey/Animation/DocumentTimeline.hpp"
#include "Krystal.Booey/Animation/AnimationEventBase.hpp"
#include "Krystal.Booey/Animation/AnimationTimelinesController.hpp"
#include "Krystal.Booey/Animation/CSSTransition.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Core/Types/HashSet.hpp"

namespace krys::boo
{
  namespace
  {
    constexpr auto Infinity = Seconds {std::numeric_limits<double>::infinity()};
  }

  Ref<DocumentTimeline> DocumentTimeline::Create(dom::Document &document) noexcept
  {
    return AdoptRef(*new DocumentTimeline(document, Seconds {0}));
  }

  Ref<DocumentTimeline> DocumentTimeline::Create(dom::Document &document,
                                                 DocumentTimelineOptions &&options) noexcept
  {
    return AdoptRef(*new DocumentTimeline(document, Seconds {options.OriginTime / 1'000}));
  }

  DocumentTimeline::DocumentTimeline(dom::Document &document, Seconds originTime) noexcept
      : _tickScheduleTimer(*this, &DocumentTimeline::ScheduleAnimationResolution),
        _document(CreateWeakPtr(&document)), _originTime(originTime)
  {
    document.EnsureTimelinesController().AddTimeline(*this);
  }

  DocumentTimeline::~DocumentTimeline() noexcept = default;

  AnimationTimelinesController *DocumentTimeline::Controller() const noexcept
  {
    if (RefPtr document = ShareRefPtr(_document.get()))
    {
      return &document->EnsureTimelinesController();
    }
    return nullptr;
  }

  void DocumentTimeline::DetachFromDocument() noexcept
  {
    AnimationTimeline::DetachFromDocument();
    _pendingAnimationEvents.clear();
    _acceleratedAnimationsPendingRunningStateChange.clear();
    ClearTickScheduleTimer();
    _document = nullptr;
  }

  Seconds DocumentTimeline::AnimationInterval() const noexcept
  {
    if (!_document || !_document->GetPage())
    {
      return Infinity;
    }
    // FIXME: The page's preferred rendering update interval, once the host reports display refreshes.
    return FullSpeedAnimationInterval;
  }

  void DocumentTimeline::SuspendAnimations() noexcept
  {
    AnimationTimeline::SuspendAnimations();

    ApplyPendingAcceleratedAnimations();

    ClearTickScheduleTimer();
  }

  void DocumentTimeline::ResumeAnimations() noexcept
  {
    AnimationTimeline::ResumeAnimations();
    ScheduleAnimationResolution();
  }

  Maybe<WebAnimationTime> DocumentTimeline::CurrentTime(UseCachedCurrentTime useCachedCurrentTime) noexcept
  {
    if (auto *controller = Controller())
    {
      if (auto currentTime = controller->CurrentTime(useCachedCurrentTime))
      {
        return WebAnimationTime {*currentTime - _originTime};
      }
      return null;
    }
    return AnimationTimeline::CurrentTime();
  }

  void DocumentTimeline::AnimationTimingDidChange(WebAnimation &animation) noexcept
  {
    AnimationTimeline::AnimationTimingDidChange(animation);
    if (!animation.IsEffectInvalidationSuspended())
    {
      ScheduleAnimationResolution();
    }
  }

  void DocumentTimeline::RemoveAnimation(WebAnimation &animation) noexcept
  {
    AnimationTimeline::RemoveAnimation(animation);
    if (_animations.isEmpty())
    {
      ClearTickScheduleTimer();
    }
  }

  void DocumentTimeline::ScheduleAnimationResolution() noexcept
  {
    if (AnimationsAreSuspended() || _animationResolutionScheduled || !_document || !_document->GetPage())
    {
      return;
    }

    // We need some relevant animations or pending events to proceed.
    if (!ShouldRunUpdateAnimationsAndSendEventsIgnoringSuspensionState())
    {
      return;
    }

    _document->GetPage()->ScheduleRenderingUpdate({RenderingUpdateStep::Animations});
    _animationResolutionScheduled = true;
  }

  void DocumentTimeline::ClearTickScheduleTimer() noexcept
  {
    _tickScheduleTimer.Stop();
  }

  bool DocumentTimeline::ShouldRunUpdateAnimationsAndSendEventsIgnoringSuspensionState() const noexcept
  {
    return !_animations.isEmpty() || !_pendingAnimationEvents.empty()
           || !_acceleratedAnimationsPendingRunningStateChange.empty();
  }

  AnimationTimeline::ShouldUpdateAnimationsAndSendEvents
    DocumentTimeline::DocumentWillUpdateAnimationsAndSendEvents() noexcept
  {
    // Updating animations and sending events may invalidate the timing of some animations, so we must set the
    // _animationResolutionScheduled flag to false prior to running that procedure to allow animation with
    // timing model updates to schedule updates.
    bool wasAnimationResolutionScheduled = std::exchange(_animationResolutionScheduled, false);

    if (!wasAnimationResolutionScheduled || AnimationsAreSuspended()
        || !ShouldRunUpdateAnimationsAndSendEventsIgnoringSuspensionState())
    {
      return ShouldUpdateAnimationsAndSendEvents::No;
    }

    // EnqueueAnimationEvent() calls ScheduleAnimationResolution() to ensure that the "update animations and
    // send events" procedure is run and enqueued events are dispatched in the next frame. However, events
    // that are enqueued while this procedure is running should not schedule animation resolution until the
    // event queue has been cleared.
    _shouldScheduleAnimationResolutionForNewPendingEvents = false;

    return ShouldUpdateAnimationsAndSendEvents::Yes;
  }

  void DocumentTimeline::AnimationAcceleratedRunningStateDidChange(WebAnimation &animation) noexcept
  {
    _acceleratedAnimationsPendingRunningStateChange.insert(ShareRefPtr(&animation));

    if (ShouldRunUpdateAnimationsAndSendEventsIgnoringSuspensionState())
    {
      ScheduleAnimationResolution();
    }
    else
    {
      ClearTickScheduleTimer();
    }
  }

  void DocumentTimeline::ApplyPendingAcceleratedAnimations() noexcept
  {
    auto acceleratedAnimationsPendingRunningStateChange =
      std::exchange(_acceleratedAnimationsPendingRunningStateChange, {});

    HashSet<KeyframeEffectStack *> keyframeEffectStacksToUpdate;

    bool hasForcedLayout = false;
    for (const auto &animation : acceleratedAnimationsPendingRunningStateChange)
    {
      if (RefPtr keyframeEffect = ShareRefPtr(animation->GetKeyframeEffect()))
      {
        if (!hasForcedLayout)
        {
          hasForcedLayout |= keyframeEffect->ForceLayoutIfNeeded();
        }

        // If the animation is no longer relevant, apply the pending accelerations in isolation.
        if (!animation->IsRelevant())
        {
          keyframeEffect->ApplyPendingAcceleratedActions();
          continue;
        }

        // Otherwise, this accelerated animation exists in the context of an effect stack and we must
        // update all effects in that stack to ensure their sort order is respected.
        if (auto target = keyframeEffect->TargetStyleable())
        {
          if (auto *keyframeEffectStack = target->GetKeyframeEffectStack())
          {
            keyframeEffectStacksToUpdate.insert(keyframeEffectStack);
          }
        }
      }
    }

    for (auto *keyframeEffectStack : keyframeEffectStacksToUpdate)
    {
      keyframeEffectStack->ApplyPendingAcceleratedActions();
    }
  }

  void DocumentTimeline::DocumentDidUpdateAnimationsAndSendEvents() noexcept
  {
    ApplyPendingAcceleratedAnimations();

    if (!_animationResolutionScheduled)
    {
      ScheduleNextTick();
    }
  }

  void DocumentTimeline::StyleOriginatedAnimationsWereCreated() noexcept
  {
    ScheduleAnimationResolution();
  }

  void DocumentTimeline::PendingStartTimeWasSetOnAnimation() noexcept
  {
    ScheduleAnimationResolution();
  }

  bool DocumentTimeline::AnimationCanBeRemoved(WebAnimation &animation) noexcept
  {
    // https://drafts.csswg.org/web-animations/#removing-replaced-animations

    krys_debug_assert(_document);

    // - is replaceable, and
    if (!animation.IsReplaceable())
    {
      return false;
    }

    // - has a replace state of active, and
    if (animation.GetReplaceState() != WebAnimation::ReplaceState::Active)
    {
      return false;
    }

    // - has an associated animation effect whose target element is a descendant of doc, and
    RefPtr keyframeEffect = ShareRefPtr(animation.GetKeyframeEffect());
    if (!keyframeEffect)
    {
      return false;
    }

    auto target = keyframeEffect->TargetStyleable();
    if (!target || !dom::TreeQueries::IsDescendant(target->Element, *_document))
    {
      return false;
    }

    const auto &style = [&]() -> const RenderStyle &
    {
      if (auto *renderer = target->Renderer())
      {
        return renderer->Style();
      }
      return RenderStyle::DefaultStyleSingleton();
    }();

    auto resolvedProperty = [&](AnimatableCSSProperty property) -> AnimatableCSSProperty
    {
      if (const auto *propertyId = std::get_if<css::PropertyId>(&property))
      {
        return css::Property::ResolveDirectionAwareProperty(*propertyId, style.GetWritingMode());
      }
      return property;
    };

    HashSet<AnimatableCSSProperty, AnimatableCSSPropertyHash> propertiesToMatch;
    for (const auto &property : keyframeEffect->AnimatedProperties())
    {
      propertiesToMatch.insert(resolvedProperty(property));
    }

    List<RefPtr<WebAnimation>> protectedAnimations;
    if (auto *effectStack = target->GetKeyframeEffectStack())
    {
      for (const auto &effect : effectStack->SortedEffects())
      {
        protectedAnimations.push_back(ShareRefPtr(effect->Animation()));
      }
    }

    for (auto it = protectedAnimations.rbegin(); it != protectedAnimations.rend(); ++it)
    {
      const auto &animationWithHigherCompositeOrder = *it;
      if (&animation == animationWithHigherCompositeOrder.get())
      {
        break;
      }

      if (animationWithHigherCompositeOrder && animationWithHigherCompositeOrder->IsReplaceable())
      {
        if (RefPtr keyframeEffectWithHigherCompositeOrder =
              ShareRefPtr(animationWithHigherCompositeOrder->GetKeyframeEffect()))
        {
          for (const auto &property : keyframeEffectWithHigherCompositeOrder->AnimatedProperties())
          {
            if (propertiesToMatch.erase(resolvedProperty(property)) && propertiesToMatch.empty())
            {
              break;
            }
          }
        }
      }
    }

    return propertiesToMatch.empty();
  }

  void DocumentTimeline::RemoveReplacedAnimations() noexcept
  {
    // https://drafts.csswg.org/web-animations/#removing-replaced-animations

    List<Ref<WebAnimation>> animationsToRemove;

    // When asked to remove replaced animations for a Document, doc, then for every animation, animation
    for (const auto &animation : _animations)
    {
      if (!AnimationCanBeRemoved(*animation))
      {
        continue;
      }

      // perform the following steps:
      // 1. Set animation's replace state to removed.
      animation->SetReplaceState(WebAnimation::ReplaceState::Removed);
      // 2. Create an AnimationPlaybackEvent, removeEvent.
      // 3. Set removeEvent's type attribute to remove.
      // 4. Set removeEvent's currentTime attribute to the current time of animation.
      // 5. Set removeEvent's timelineTime attribute to the current time of the timeline with which animation
      // is
      //    associated.
      // 6. If animation has a document for timing, then append removeEvent to its document for timing's
      // pending
      //    animation event queue along with its target, animation. For the scheduled event time, use the
      //    result of applying the procedure to convert timeline time to origin-relative time to the current
      //    time of the timeline with which animation is associated. Otherwise, queue a task to dispatch
      //    removeEvent at animation. The task source for this task is the DOM manipulation task source.
      auto scheduledTime = [&]() -> Maybe<WebAnimationTime>
      {
        if (auto *documentTimeline = DynamicDowncast<DocumentTimeline>(animation->Timeline()))
        {
          auto currentTime = Seconds {html::EventLoop::Current().Now()};
          return WebAnimationTime {documentTimeline->ConvertTimelineTimeToOriginRelativeTime(currentTime)};
        }
        return null;
      }();

      auto animationCurrentTime = [&]() -> Maybe<WebAnimationTime>
      {
        if (auto animationTime = animation->CurrentTime())
        {
          if (auto time = animationTime->Time())
          {
            return WebAnimationTime {*time};
          }
        }
        return null;
      }();

      animation->EnqueueAnimationPlaybackEvent(EventNames::Remove, animationCurrentTime, scheduledTime);

      animationsToRemove.push_back(animation);
    }

    for (const auto &animation : animationsToRemove)
    {
      if (RefPtr timeline = ShareRefPtr(animation->Timeline()))
      {
        timeline->RemoveAnimation(*animation);
      }
    }
  }

  void DocumentTimeline::TransitionDidComplete(Ref<CSSTransition> &&transition) noexcept
  {
    RemoveAnimation(*transition);
    if (RefPtr keyframeEffect = ShareRefPtr(transition->GetKeyframeEffect()))
    {
      if (auto styleable = keyframeEffect->TargetStyleable())
      {
        auto property = transition->Property();
        if (styleable->HasRunningTransitionForProperty(property))
        {
          styleable->EnsureCompletedTransitionsByProperty().insert_or_assign(property,
                                                                             krys::move(transition));
        }
      }
    }
  }

  void DocumentTimeline::ScheduleNextTick() noexcept
  {
    // If we have pending animation events, we need to schedule an update right away.
    if (!_pendingAnimationEvents.empty())
    {
      ScheduleAnimationResolution();
    }

    // There is no tick to schedule if we don't have any relevant animations.
    if (_animations.isEmpty())
    {
      return;
    }

    Seconds scheduleDelay = Infinity;

    auto animationInterval = AnimationInterval();
    for (const auto &animation : _animations)
    {
      if (!animation->IsRelevant())
      {
        continue;
      }

      // Get the time until the next tick for this animation. This does not account for the animation frame
      // rate, only accounting for the timing model and the playback rate.
      auto animationTimeToNextRequiredTick = animation->TimeToNextTick();
      if (animationTimeToNextRequiredTick < animationInterval)
      {
        ScheduleAnimationResolution();
        return;
      }

      scheduleDelay = std::min(scheduleDelay, animationTimeToNextRequiredTick);
    }

    if (scheduleDelay < Infinity)
    {
      scheduleDelay = std::max(scheduleDelay - animationInterval, Seconds {0});
      _tickScheduleTimer.StartOneShot(std::chrono::duration_cast<MonotonicTime::duration>(scheduleDelay));
    }
  }

  void DocumentTimeline::EnqueueAnimationEvent(AnimationEventBase &event) noexcept
  {
    _pendingAnimationEvents.push_back(ShareRef(event));
    if (_shouldScheduleAnimationResolutionForNewPendingEvents)
    {
      ScheduleAnimationResolution();
    }
  }

  bool DocumentTimeline::HasPendingAnimationEventForAnimation(const WebAnimation &animation) const noexcept
  {
    for (const auto &event : _pendingAnimationEvents)
    {
      if (event->Animation() == &animation)
      {
        return true;
      }
    }
    return false;
  }

  AnimationEvents DocumentTimeline::PrepareForPendingAnimationEventsDispatch() noexcept
  {
    _shouldScheduleAnimationResolutionForNewPendingEvents = true;
    return std::exchange(_pendingAnimationEvents, {});
  }

  Seconds DocumentTimeline::ConvertTimelineTimeToOriginRelativeTime(Seconds timelineTime) const noexcept
  {
    // https://drafts.csswg.org/web-animations-1/#ref-for-timeline-time-to-origin-relative-time
    return timelineTime + _originTime;
  }
}

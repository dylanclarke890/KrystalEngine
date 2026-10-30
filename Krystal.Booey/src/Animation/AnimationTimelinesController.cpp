#include "Krystal.Booey/Animation/AnimationTimelinesController.hpp"
#include "Krystal.Booey/Animation/AnimationEventBase.hpp"
#include "Krystal.Booey/Animation/CSSTransition.hpp"
#include "Krystal.Booey/Animation/DocumentTimeline.hpp"
#include "Krystal.Booey/Animation/ScrollTimeline.hpp"
#include "Krystal.Booey/Animation/StyleOriginatedTimelinesController.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Animation/WebAnimationUtilities.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include <algorithm>

namespace krys::boo
{
  AnimationTimelinesController::AnimationTimelinesController(dom::Document &document) noexcept
      : _cachedCurrentTimeClearanceTimer(*this, &AnimationTimelinesController::ClearCachedCurrentTime),
        _document(document)
  {
    // FIXME: Suspending animations in hidden pages, once pages have a visibility.
  }

  AnimationTimelinesController::~AnimationTimelinesController() noexcept = default;

  void AnimationTimelinesController::AddTimeline(AnimationTimeline &timeline) noexcept
  {
    _timelines.push_back(CreateWeakPtr(&timeline));
    if (_isSuspended)
    {
      timeline.SuspendAnimations();
    }
    else
    {
      timeline.ResumeAnimations();
    }
  }

  void AnimationTimelinesController::RemoveTimeline(AnimationTimeline &timeline) noexcept
  {
    std::erase_if(_timelines,
                  [&](const auto &weakTimeline) { return !weakTimeline || weakTimeline.get() == &timeline; });
  }

  void AnimationTimelinesController::DetachFromDocument() noexcept
  {
    CancelPendingAnimationsProcessingTask();

    while (!_timelines.empty())
    {
      auto weakTimeline = _timelines.back();
      _timelines.pop_back();
      if (RefPtr timeline = ShareRefPtr(weakTimeline.get()))
      {
        timeline->DetachFromDocument();
      }
    }
  }

  void AnimationTimelinesController::AddPendingAnimation(WebAnimation &animation) noexcept
  {
    for (const auto &pendingAnimation : _pendingAnimations)
    {
      if (pendingAnimation.get() == &animation)
      {
        return;
      }
    }
    _pendingAnimations.push_back(CreateWeakPtr(&animation));
  }

  void AnimationTimelinesController::UpdateAnimationsAndSendEvents(Seconds timestamp) noexcept
  {
    // We need to copy _timelines before iterating over its members since the steps in this procedure may
    // mutate _timelines.
    List<Ref<AnimationTimeline>> protectedTimelines;
    for (const auto &timeline : _timelines)
    {
      if (timeline)
      {
        protectedTimelines.push_back(ShareRef(*timeline));
      }
    }

    // We need to freeze the current time even if no animation is running. document.timeline.currentTime may
    // be called from a rAF callback and it has to match the rAF timestamp.
    if (!_isSuspended)
    {
      CacheCurrentTime(timestamp);
    }

    // 1. Update the current time of all timelines associated with document passing now as the timestamp.
    krys_debug_assert(_updatedScrollTimelines.empty());
    List<Ref<AnimationTimeline>> timelinesToUpdate;
    List<Ref<WebAnimation>> animationsToRemove;
    List<Ref<CSSTransition>> completedTransitions;

    for (auto &timeline : protectedTimelines)
    {
      auto shouldUpdateAnimationsAndSendEvents = timeline->DocumentWillUpdateAnimationsAndSendEvents();
      if (shouldUpdateAnimationsAndSendEvents == AnimationTimeline::ShouldUpdateAnimationsAndSendEvents::No)
      {
        continue;
      }

      timelinesToUpdate.push_back(timeline);

      // https://drafts.csswg.org/scroll-animations-1/#event-loop
      if (auto *scrollTimeline = DynamicDowncast<ScrollTimeline>(timeline.get()))
      {
        _updatedScrollTimelines.push_back(ShareRef(*scrollTimeline));
      }

      List<Ref<WebAnimation>> relevantAnimations(timeline->RelevantAnimations().begin(),
                                                 timeline->RelevantAnimations().end());
      for (auto &animation : relevantAnimations)
      {
        if (animation->IsSkippedContentAnimation())
        {
          continue;
        }

        if (animation->Timeline() != timeline.get())
        {
          krys_debug_assert(!animation->Timeline());
          continue;
        }

        // FIXME: Skipping animations whose frame rate doesn't align with this update, once there are
        // animation frame rates.

        // This will notify the animation that timing has changed and will call automatically schedule
        // invalidation if required for this animation.
        animation->Tick();

        if (!animation->IsRelevant() && !animation->NeedsTick() && !IsPendingTimelineAttachment(*animation))
        {
          animationsToRemove.push_back(animation);
        }

        if (auto *transition = DynamicDowncast<CSSTransition>(animation.get()))
        {
          if (!transition->NeedsTick() && transition->GetPlayState() == WebAnimation::PlayState::Finished
              && transition->OwningElement())
          {
            completedTransitions.push_back(ShareRef(*transition));
          }
        }
      }
    }

    if (timelinesToUpdate.empty())
    {
      return;
    }

    // 2. Remove replaced animations for document.
    for (auto &timeline : protectedTimelines)
    {
      if (auto *documentTimeline = DynamicDowncast<DocumentTimeline>(timeline.get()))
      {
        documentTimeline->RemoveReplacedAnimations();
      }
    }

    // 3. Perform a microtask checkpoint.
    html::EventLoop::Current().PerformMicrotaskCheckpoint();

    if (RefPtr documentTimeline = ShareRefPtr(_document.ExistingTimeline()))
    {
      // FIXME: pending animation events should be owned by this controller rather than the document timeline.

      // 4. Let events to dispatch be a copy of doc's pending animation event queue.
      // 5. Clear doc's pending animation event queue.
      auto events = documentTimeline->PrepareForPendingAnimationEventsDispatch();

      // 6. Perform a stable sort of the animation events in events to dispatch as follows.
      std::ranges::stable_sort(events, [](const auto &a, const auto &b)
                               { return CompareAnimationEventsByCompositeOrder(*a, *b); });

      // 7. Dispatch each of the events in events to dispatch at their corresponding target using the order
      //    established in the previous step.
      for (auto &event : events)
      {
        RefPtr target = event->Target();
        (void)dom::EventDispatcher::DispatchToTarget(*event, target.get());
      }
    }

    // This will cancel any scheduled invalidation if we end up removing all animations.
    for (auto &animation : animationsToRemove)
    {
      // An animation that was initially marked as irrelevant may have changed while we were sending events,
      // so redo the the check for whether it should be removed.
      if (RefPtr timeline = ShareRefPtr(animation->Timeline()))
      {
        if (!animation->IsRelevant() && !animation->NeedsTick())
        {
          timeline->RemoveAnimation(*animation);
        }
      }
    }

    // Now that animations that needed removal have been removed, update the list of completed transitions.
    // This needs to happen after dealing with the list of animations to remove as the animation may have been
    // removed from the list of completed transitions otherwise.
    for (auto &completedTransition : completedTransitions)
    {
      if (RefPtr documentTimeline =
            ShareRefPtr(DynamicDowncast<DocumentTimeline>(completedTransition->Timeline())))
      {
        documentTimeline->TransitionDidComplete(krys::move(completedTransition));
      }
    }

    for (auto &timeline : timelinesToUpdate)
    {
      if (auto *documentTimeline = DynamicDowncast<DocumentTimeline>(timeline.get()))
      {
        documentTimeline->DocumentDidUpdateAnimationsAndSendEvents();
      }
    }
  }

  void AnimationTimelinesController::RunPostRenderingUpdateTasks() noexcept
  {
    // https://drafts.csswg.org/scroll-animations-1/#event-loop
    auto scrollTimelines = std::exchange(_updatedScrollTimelines, {});
    for (auto &scrollTimeline : scrollTimelines)
    {
      scrollTimeline->UpdateCurrentTimeIfStale();
    }
  }

  void AnimationTimelinesController::SuspendAnimations() noexcept
  {
    if (_isSuspended)
    {
      return;
    }

    if (!_cachedCurrentTime)
    {
      _cachedCurrentTime = LiveCurrentTime();
    }

    _cachedCurrentTimeClearanceTimer.Stop();

    for (const auto &timeline : _timelines)
    {
      if (RefPtr protectedTimeline = ShareRefPtr(timeline.get()))
      {
        protectedTimeline->SuspendAnimations();
      }
    }

    _isSuspended = true;
  }

  void AnimationTimelinesController::ResumeAnimations() noexcept
  {
    if (!_isSuspended)
    {
      return;
    }

    _isSuspended = false;
    ClearCachedCurrentTime();

    for (const auto &timeline : _timelines)
    {
      if (RefPtr protectedTimeline = ShareRefPtr(timeline.get()))
      {
        protectedTimeline->ResumeAnimations();
      }
    }
  }

  Seconds AnimationTimelinesController::LiveCurrentTime() const noexcept
  {
    return Seconds {html::EventLoop::Current().Now()};
  }

  Maybe<Seconds> AnimationTimelinesController::CurrentTime(UseCachedCurrentTime useCachedCurrentTime) noexcept
  {
    if (!_document.Frame())
    {
      return null;
    }

    if (useCachedCurrentTime == UseCachedCurrentTime::No && !_isSuspended)
    {
      return LiveCurrentTime();
    }

    if (!_cachedCurrentTime)
    {
      CacheCurrentTime(LiveCurrentTime());
    }

    return *_cachedCurrentTime;
  }

  void AnimationTimelinesController::CacheCurrentTime(Seconds newCurrentTime) noexcept
  {
    if (_cachedCurrentTime == newCurrentTime)
    {
      return;
    }

    _cachedCurrentTimeClearanceTimer.Stop();

    // We can get in a situation where the event loop will not run a task that had been enqueued. If that is
    // the case, we must clear the task group and run the callback prior to adding a new task.
    if (_pendingAnimationsProcessingTaskIsQueued && _cachedCurrentTime)
    {
      CancelPendingAnimationsProcessingTask();
      ProcessPendingAnimations();
    }

    _cachedCurrentTime = newCurrentTime;
    _lastCachedCurrentTime = newCurrentTime;

    // As we've advanced to a new current time, we want all animations created during this run loop to have
    // this newly-cached current time as their start time. To that end, we schedule a task to set that current
    // time on all animations created until then as their pending start time.
    if (!_pendingAnimationsProcessingTaskIsQueued)
    {
      _pendingAnimationsProcessingTaskIsQueued = true;
      html::EventLoop::Current().QueueTask(
        html::TaskSource::InternalAsyncTask, _document,
        [document = CreateWeakPtr(&_document), identifier = _pendingAnimationsProcessingTaskIdentifier]
        {
          if (!document)
          {
            return;
          }
          auto *controller = document->TimelinesController();
          if (!controller || !controller->_pendingAnimationsProcessingTaskIsQueued
              || controller->_pendingAnimationsProcessingTaskIdentifier != identifier)
          {
            return;
          }
          controller->_pendingAnimationsProcessingTaskIsQueued = false;
          controller->ProcessPendingAnimations();
        });
    }

    if (!_isSuspended)
    {
      // In order to not have a stale cached current time, we schedule a timer to reset it in the time it
      // would take an animation frame to run under normal circumstances.
      // FIXME: The page's preferred rendering update interval, once the host reports display refreshes.
      _cachedCurrentTimeClearanceTimer.StartOneShot(
        std::chrono::duration_cast<MonotonicTime::duration>(FullSpeedAnimationInterval));
    }
  }

  void AnimationTimelinesController::ClearCachedCurrentTime() noexcept
  {
    krys_debug_assert(!_isSuspended);
    _cachedCurrentTime = null;
  }

  void AnimationTimelinesController::CancelPendingAnimationsProcessingTask() noexcept
  {
    _pendingAnimationsProcessingTaskIsQueued = false;
    ++_pendingAnimationsProcessingTaskIdentifier;
  }

  void AnimationTimelinesController::ProcessPendingAnimations() noexcept
  {
    if (_isSuspended || !_lastCachedCurrentTime)
    {
      return;
    }

    krys_debug_assert(!_pendingAnimationsProcessingTaskIsQueued);

    auto pendingAnimations = std::exchange(_pendingAnimations, {});
    for (const auto &weakAnimation : pendingAnimations)
    {
      RefPtr pendingAnimation = ShareRefPtr(weakAnimation.get());
      if (!pendingAnimation)
      {
        continue;
      }
      if (pendingAnimation->Timeline() && pendingAnimation->Timeline()->IsMonotonic())
      {
        pendingAnimation->SetPendingStartTime(WebAnimationTime {*_lastCachedCurrentTime});
      }
    }
  }

  bool AnimationTimelinesController::IsPendingTimelineAttachment(const WebAnimation &animation) const noexcept
  {
    auto *styleOriginatedTimelinesController = _document.GetStyleOriginatedTimelinesController();
    return styleOriginatedTimelinesController
           && styleOriginatedTimelinesController->IsPendingTimelineAttachment(animation);
  }
}

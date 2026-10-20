#pragma once

#include "Krystal.Booey/Animation/WebAnimationTime.hpp"
#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class AnimationTimeline;
  class WebAnimation;

  /// @brief A document's timelines: it caches the current time for an animation frame, gives pending
  /// animations their start time, and runs the "update animations and send events" procedure.
  // FIXME: Frame rate alignment, accelerated effect stacks, scroll timelines and hidden page suspension, once
  // there are any.
  class AnimationTimelinesController final
  {
    KRYS_NON_COPY_MOVABLE(AnimationTimelinesController);

  private:
    Timer _cachedCurrentTimeClearanceTimer;
    List<WeakPtr<AnimationTimeline>> _timelines;
    List<WeakPtr<WebAnimation>> _pendingAnimations;
    dom::Document &_document;
    Maybe<Seconds> _cachedCurrentTime;
    /// @brief The last time cached, which the clearance timer doesn't clear: the pending animations' start
    /// time. The host runs the event loop once per frame, timers first, so the cache is usually cleared by
    /// the time the task processing the pending animations runs.
    Maybe<Seconds> _lastCachedCurrentTime;
    /// @brief Identifies the queued task processing pending animations; changing it cancels the task.
    uint64 _pendingAnimationsProcessingTaskIdentifier {0};
    bool _pendingAnimationsProcessingTaskIsQueued {false};
    bool _isSuspended {false};

  public:
    explicit AnimationTimelinesController(dom::Document &document) noexcept;
    ~AnimationTimelinesController() noexcept;

    void AddTimeline(AnimationTimeline &timeline) noexcept;
    void RemoveTimeline(AnimationTimeline &timeline) noexcept;
    void DetachFromDocument() noexcept;

    /// @see https://drafts.csswg.org/web-animations-1/#update-animations-and-send-events
    void UpdateAnimationsAndSendEvents(Seconds timestamp) noexcept;

    void AddPendingAnimation(WebAnimation &animation) noexcept;

    KRYS_NODISCARD Maybe<Seconds>
      CurrentTime(UseCachedCurrentTime useCachedCurrentTime = UseCachedCurrentTime::Yes) noexcept;

    void SuspendAnimations() noexcept;
    void ResumeAnimations() noexcept;
    KRYS_NODISCARD bool AnimationsAreSuspended() const noexcept
    {
      return _isSuspended;
    }

  private:
    KRYS_NODISCARD Seconds LiveCurrentTime() const noexcept;
    void CacheCurrentTime(Seconds newCurrentTime) noexcept;
    void ClearCachedCurrentTime() noexcept;
    void ProcessPendingAnimations() noexcept;
    void CancelPendingAnimationsProcessingTask() noexcept;
  };
}

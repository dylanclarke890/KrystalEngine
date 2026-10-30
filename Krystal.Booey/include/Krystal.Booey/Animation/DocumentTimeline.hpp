#pragma once

#include "Krystal.Booey/Animation/AnimationTimeline.hpp"
#include "Krystal.Booey/Animation/DocumentTimelineOptions.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class AnimationEventBase;
  class CSSTransition;

  using AnimationEvents = List<Ref<AnimationEventBase>>;

  /// @brief The default timeline of a document: its time is the host's time since the document's time origin.
  /// @see https://drafts.csswg.org/web-animations-1/#the-documents-default-timeline
  // FIXME: Accelerated timelines (threaded animations), once there are any.
  class DocumentTimeline final : public AnimationTimeline
  {
  private:
    Timer _tickScheduleTimer;
    AnimationEvents _pendingAnimationEvents;
    HashSet<RefPtr<WebAnimation>> _acceleratedAnimationsPendingRunningStateChange;
    WeakPtr<dom::Document> _document;
    Seconds _originTime;
    bool _animationResolutionScheduled {false};
    bool _shouldScheduleAnimationResolutionForNewPendingEvents {true};

    DocumentTimeline(dom::Document &document, Seconds originTime) noexcept;

  public:
    ~DocumentTimeline() noexcept override;

    KRYS_NODISCARD static Ref<DocumentTimeline> Create(dom::Document &document) noexcept;
    /// @brief The `DocumentTimeline(options)` constructor: a timeline whose zero time is the options' origin
    /// time.
    /// @see https://drafts.csswg.org/web-animations-1/#dom-documenttimeline-documenttimeline
    KRYS_NODISCARD static Ref<DocumentTimeline> Create(dom::Document &document,
                                                       DocumentTimelineOptions &&options) noexcept;

    KRYS_NODISCARD dom::Document *Document() const noexcept
    {
      return _document.get();
    }

    KRYS_NODISCARD Maybe<WebAnimationTime>
      CurrentTime(UseCachedCurrentTime useCachedCurrentTime = UseCachedCurrentTime::Yes) noexcept override;

    void AnimationTimingDidChange(WebAnimation &animation) noexcept override;
    void RemoveAnimation(WebAnimation &animation) noexcept override;
    void TransitionDidComplete(Ref<CSSTransition> &&transition) noexcept;
    void AnimationAcceleratedRunningStateDidChange(WebAnimation &animation) noexcept;

    void DetachFromDocument() noexcept override;

    void EnqueueAnimationEvent(AnimationEventBase &event) noexcept;
    KRYS_NODISCARD bool HasPendingAnimationEventForAnimation(const WebAnimation &animation) const noexcept;

    KRYS_NODISCARD ShouldUpdateAnimationsAndSendEvents
      DocumentWillUpdateAnimationsAndSendEvents() noexcept override;
    /// @see https://drafts.csswg.org/web-animations/#removing-replaced-animations
    void RemoveReplacedAnimations() noexcept;
    KRYS_NODISCARD AnimationEvents PrepareForPendingAnimationEventsDispatch() noexcept;
    void DocumentDidUpdateAnimationsAndSendEvents() noexcept;
    void StyleOriginatedAnimationsWereCreated() noexcept;
    void PendingStartTimeWasSetOnAnimation() noexcept;

    /// @brief The interval between the document's rendering updates.
    KRYS_NODISCARD Seconds AnimationInterval() const noexcept;

    void SuspendAnimations() noexcept override;
    void ResumeAnimations() noexcept override;

    /// @see https://drafts.csswg.org/web-animations-1/#ref-for-timeline-time-to-origin-relative-time
    KRYS_NODISCARD Seconds ConvertTimelineTimeToOriginRelativeTime(Seconds timelineTime) const noexcept;

  private:
    KRYS_NODISCARD bool IsDocumentTimeline() const noexcept final
    {
      return true;
    }
    KRYS_NODISCARD AnimationTimelinesController *Controller() const noexcept override;

    void ScheduleAnimationResolution() noexcept;
    void ClearTickScheduleTimer() noexcept;
    void ScheduleNextTick() noexcept;
    void ApplyPendingAcceleratedAnimations() noexcept;
    KRYS_NODISCARD bool AnimationCanBeRemoved(WebAnimation &animation) noexcept;
    KRYS_NODISCARD bool ShouldRunUpdateAnimationsAndSendEventsIgnoringSuspensionState() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_ANIMATION_TIMELINE(krys::boo::DocumentTimeline, IsDocumentTimeline())

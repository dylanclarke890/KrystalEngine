#pragma once

#include "Krystal.Booey/Animation/AnimationEffect.hpp"
#include "Krystal.Booey/Animation/AnimationPromise.hpp"
#include "Krystal.Booey/Animation/AnimationTimeline.hpp"
#include "Krystal.Booey/Animation/WebAnimationTime.hpp"
#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/HashSet.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::style
{
  struct ResolutionContext;
  struct Styleable;
}

namespace krys::boo
{
  class AnimationEffect;
  class AnimationEventBase;
  class AnimationTimeline;
  class KeyframeEffect;
  class RenderStyle;

  /// @brief An animation: it plays an animation effect, and gives it its time from a timeline.
  /// @see https://drafts.csswg.org/web-animations-1/#animations
  // FIXME: The animation range (`rangeStart`, `rangeEnd`), once there are scroll timelines, and suspending
  // animations with the document (upstream's `ActiveDOMObject`).
  class WebAnimation : public dom::EventTarget
  {
  public:
    enum class PlayState : uint8
    {
      Idle,
      Running,
      Paused,
      Finished
    };

    enum class ReplaceState : uint8
    {
      Active,
      Removed,
      Persisted
    };

    enum class Silently : bool
    {
      No,
      Yes
    };

  private:
    enum class DidSeek : bool
    {
      No,
      Yes
    };
    enum class SynchronouslyNotify : bool
    {
      No,
      Yes
    };
    enum class RespectHoldTime : bool
    {
      No,
      Yes
    };
    enum class AutoRewind : bool
    {
      No,
      Yes
    };
    enum class TimeToRunPendingTask : uint8
    {
      NotScheduled,
      ASAP,
      WhenReady
    };

    RefPtr<AnimationEffect> _effect;
    RefPtr<AnimationTimeline> _timeline;
    Ref<AnimationPromise> _readyPromise;
    Ref<AnimationPromise> _finishedPromise;
    Maybe<WebAnimationTime> _previousCurrentTime;
    Maybe<WebAnimationTime> _startTime;
    Maybe<WebAnimationTime> _pendingStartTime;
    Maybe<WebAnimationTime> _holdTime;
    Maybe<double> _pendingPlaybackRate;
    double _playbackRate {1};
    dom::DOMString _id;
    int32 _suspendCount {0};
    bool _isSuspended {false};
    bool _finishNotificationStepsMicrotaskPending {false};
    bool _isRelevant {false};
    bool _shouldSkipUpdatingFinishedStateWhenResolving {false};
    bool _hasScheduledEventsDuringTick {false};
    bool _autoAlignStartTime {false};
    TimeToRunPendingTask _timeToRunPendingPlayTask {TimeToRunPendingTask::NotScheduled};
    TimeToRunPendingTask _timeToRunPendingPauseTask {TimeToRunPendingTask::NotScheduled};
    ReplaceState _replaceState {ReplaceState::Active};
    uint64 _globalPosition {0};
    WeakPtr<dom::Document> _document;

  protected:
    explicit WebAnimation(dom::Document &document) noexcept;

  public:
    ~WebAnimation() noexcept override;

    /// @brief An animation of @p effect on @p document's default timeline.
    KRYS_NODISCARD static Ref<WebAnimation> Create(dom::Document &document, AnimationEffect *effect) noexcept;
    KRYS_NODISCARD static Ref<WebAnimation> Create(dom::Document &document, AnimationEffect *effect,
                                                   AnimationTimeline *timeline) noexcept;

    /// @brief Every animation that exists.
    KRYS_NODISCARD static HashSet<WebAnimation *> &Instances() noexcept;

    KRYS_NODISCARD virtual bool IsStyleOriginatedAnimation() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsCSSAnimation() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsCSSTransition() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool IsSkippedContentAnimation() const noexcept;

    /// @see https://drafts.csswg.org/web-animations-1/#dom-animation-id
    KRYS_NODISCARD const dom::DOMString &Id() const noexcept
    {
      return _id;
    }
    void SetId(dom::DOMString &&id) noexcept;

    KRYS_NODISCARD AnimationEffect *BindingsEffect() const noexcept
    {
      return Effect();
    }
    /// @brief Sets the effect, as `effect` does.
    virtual void SetBindingsEffect(RefPtr<AnimationEffect> &&newEffect) noexcept;
    KRYS_NODISCARD AnimationEffect *Effect() const noexcept
    {
      return _effect.get();
    }
    /// @see https://drafts.csswg.org/web-animations-1/#setting-the-associated-effect
    void SetEffect(RefPtr<AnimationEffect> &&newEffect) noexcept;
    KRYS_NODISCARD KeyframeEffect *GetKeyframeEffect() const noexcept;

    KRYS_NODISCARD virtual AnimationTimeline *BindingsTimeline() const noexcept
    {
      return Timeline();
    }
    /// @brief Sets the timeline, as `timeline` does.
    virtual void SetBindingsTimeline(RefPtr<AnimationTimeline> &&timeline) noexcept;
    KRYS_NODISCARD AnimationTimeline *Timeline() const noexcept
    {
      return _timeline.get();
    }
    /// @see https://drafts.csswg.org/web-animations-1/#setting-the-timeline
    virtual void SetTimeline(RefPtr<AnimationTimeline> &&timeline) noexcept;

    /// @see https://drafts.csswg.org/web-animations-1/#the-current-time-of-an-animation
    KRYS_NODISCARD Maybe<WebAnimationTime>
      CurrentTime(UseCachedCurrentTime useCachedCurrentTime = UseCachedCurrentTime::Yes) const noexcept;
    /// @see https://drafts.csswg.org/web-animations-2/#setting-the-current-time-of-an-animation
    dom::ExceptionOr<void> SetCurrentTime(Maybe<WebAnimationTime> seekTime) noexcept;

    KRYS_NODISCARD double PlaybackRate() const noexcept
    {
      return _playbackRate + 0;
    }
    /// @see https://drafts.csswg.org/web-animations-1/#setting-the-playback-rate-of-an-animation
    void SetPlaybackRate(double newPlaybackRate) noexcept;

    /// @see https://drafts.csswg.org/web-animations/#play-states
    KRYS_NODISCARD PlayState GetPlayState() const noexcept;

    KRYS_NODISCARD ReplaceState GetReplaceState() const noexcept
    {
      return _replaceState;
    }
    void SetReplaceState(ReplaceState replaceState) noexcept;

    /// @see https://drafts.csswg.org/web-animations-1/#pending-play-task
    KRYS_NODISCARD bool Pending() const noexcept
    {
      return HasPendingPauseTask() || HasPendingPlayTask();
    }

    /// @see https://drafts.csswg.org/web-animations-1/#current-ready-promise
    KRYS_NODISCARD AnimationPromise &Ready() noexcept
    {
      return *_readyPromise;
    }

    /// @see https://drafts.csswg.org/web-animations-1/#current-finished-promise
    KRYS_NODISCARD AnimationPromise &Finished() noexcept
    {
      return *_finishedPromise;
    }

    /// @see https://drafts.csswg.org/web-animations-1/#cancel-an-animation
    virtual void Cancel(Silently silently = Silently::No) noexcept;
    /// @see https://drafts.csswg.org/web-animations-1/#finish-an-animation
    dom::ExceptionOr<void> Finish() noexcept;
    /// @see https://drafts.csswg.org/web-animations-2/#play-an-animation
    dom::ExceptionOr<void> Play() noexcept;
    /// @see https://drafts.csswg.org/web-animations/#seamlessly-update-the-playback-rate
    void UpdatePlaybackRate(double newPlaybackRate) noexcept;
    /// @see https://drafts.csswg.org/web-animations-2/#pause-an-animation
    dom::ExceptionOr<void> Pause() noexcept;
    /// @brief Reverses the animation, as `reverse()` does.
    virtual dom::ExceptionOr<void> BindingsReverse() noexcept;
    /// @see https://drafts.csswg.org/web-animations-1/#reverse-an-animation
    dom::ExceptionOr<void> Reverse() noexcept;
    /// @see https://drafts.csswg.org/web-animations-1/#dom-animation-persist
    void Persist() noexcept;
    /// @brief Writes the animation's current values into its target's `style` attribute.
    /// @return A `NoModificationAllowedError` if the target can't have a style attribute (a pseudo-element),
    /// or an `InvalidStateError` if it isn't rendered.
    /// @see https://drafts.csswg.org/web-animations-1/#dom-animation-commitstyles
    dom::ExceptionOr<void> CommitStyles() noexcept;

    /// @brief The start time, as `startTime` gives it: the style of a style-originated animation is brought
    /// up to date first.
    KRYS_NODISCARD virtual Maybe<WebAnimationTime> BindingsStartTime() const noexcept
    {
      return StartTime();
    }
    /// @return A `TypeError` if @p startTime isn't valid for the timeline: a percentage on a monotonic
    /// timeline, or a time on a progress-based one.
    virtual dom::ExceptionOr<void> SetBindingsStartTime(const Maybe<WebAnimationTime> &startTime) noexcept;
    KRYS_NODISCARD virtual Maybe<WebAnimationTime> BindingsCurrentTime() const noexcept
    {
      return CurrentTime();
    }
    /// @return A `TypeError` if @p currentTime isn't valid for the timeline, or if it's unresolved while the
    /// current time is resolved.
    virtual dom::ExceptionOr<void>
      SetBindingsCurrentTime(const Maybe<WebAnimationTime> &currentTime) noexcept;
    KRYS_NODISCARD virtual PlayState BindingsPlayState() const noexcept
    {
      return GetPlayState();
    }
    KRYS_NODISCARD virtual ReplaceState BindingsReplaceState() const noexcept
    {
      return GetReplaceState();
    }
    KRYS_NODISCARD virtual bool BindingsPending() const noexcept
    {
      return Pending();
    }
    KRYS_NODISCARD virtual AnimationPromise &BindingsReady() noexcept
    {
      return Ready();
    }
    KRYS_NODISCARD virtual AnimationPromise &BindingsFinished() noexcept
    {
      return Finished();
    }
    /// @brief Plays the animation, as `play()` does.
    virtual dom::ExceptionOr<void> BindingsPlay() noexcept
    {
      return Play();
    }
    /// @brief Pauses the animation, as `pause()` does.
    virtual dom::ExceptionOr<void> BindingsPause() noexcept
    {
      return Pause();
    }

    KRYS_NODISCARD Maybe<WebAnimationTime> StartTime() const noexcept
    {
      return _startTime;
    }
    /// @see https://drafts.csswg.org/web-animations-2/#setting-the-start-time-of-an-animation
    void SetStartTime(Maybe<WebAnimationTime> newStartTime) noexcept;

    /// @see https://drafts.csswg.org/web-animations-2/#the-overall-progress-of-an-animation
    KRYS_NODISCARD Maybe<double> OverallProgress() const noexcept;

    KRYS_NODISCARD Maybe<WebAnimationTime> HoldTime() const noexcept
    {
      return _holdTime;
    }

    void SetPendingStartTime(WebAnimationTime pendingStartTime) noexcept;

    KRYS_NODISCARD bool NeedsTick() const noexcept;
    virtual void Tick() noexcept;
    /// @brief How long until the animation next needs updating.
    KRYS_NODISCARD Seconds TimeToNextTick() const noexcept;

    /// @brief Applies the animation's effect to @p targetStyle.
    EnumSet<AnimationImpact> Resolve(RenderStyle &targetStyle,
                                     const style::ResolutionContext &resolutionContext,
                                     EndpointInclusiveActiveInterval endpointInclusiveActiveInterval =
                                       EndpointInclusiveActiveInterval::No) noexcept;

    void EffectTargetDidChange(const Maybe<style::Styleable> &previousTarget,
                               const Maybe<style::Styleable> &newTarget) noexcept;
    void WillChangeRenderer() noexcept;

    /// @see https://drafts.csswg.org/web-animations-1/#relevant-animations-section
    KRYS_NODISCARD bool IsRelevant() const noexcept
    {
      return _isRelevant;
    }
    KRYS_NODISCARD bool HasPendingFinishNotification() const noexcept
    {
      return _finishNotificationStepsMicrotaskPending;
    }
    void UpdateRelevance() noexcept;
    void EffectTimingDidChange() noexcept;

    /// @brief Stops changes to the animation from invalidating its effect's target's style, while it's being
    /// created during style resolution.
    void SuspendEffectInvalidation() noexcept;
    void UnsuspendEffectInvalidation() noexcept;
    KRYS_NODISCARD bool IsEffectInvalidationSuspended() const noexcept
    {
      return _suspendCount;
    }

    void SetSuspended(bool isSuspended) noexcept;
    /// @brief Tells the timeline the effect's accelerated actions changed, to apply them at the rendering
    /// update.
    void AcceleratedStateDidChange() noexcept;
    KRYS_NODISCARD bool IsSuspended() const noexcept
    {
      return _isSuspended;
    }

    /// @see https://drafts.csswg.org/web-animations/#removing-replaced-animations
    KRYS_NODISCARD bool IsReplaceable() const noexcept;

    /// @brief Disassociates the animation from its effect and timeline.
    void Remove() noexcept;

    void EnqueueAnimationPlaybackEvent(dom::DOMStringAtom type, Maybe<WebAnimationTime> currentTime,
                                       Maybe<WebAnimationTime> scheduledTime) noexcept;

    /// @brief The animation's position in the global animation list, or zero if it has none.
    /// @see https://drafts.csswg.org/web-animations-1/#animation-composite-order
    KRYS_NODISCARD uint64 GlobalPosition() const noexcept
    {
      return _globalPosition;
    }
    void SetGlobalPosition(uint64 globalPosition) noexcept
    {
      _globalPosition = globalPosition;
    }
    KRYS_NODISCARD virtual bool CanHaveGlobalPosition() noexcept
    {
      return true;
    }

    /// @see https://drafts.csswg.org/web-animations-1/#animation-time-to-timeline-time
    KRYS_NODISCARD Maybe<Seconds> ConvertAnimationTimeToTimelineTime(Seconds animationTime) const noexcept;

  protected:
    /// @brief Resolves the first ready promise.
    void Initialize() noexcept;
    void EnqueueAnimationEvent(Ref<AnimationEventBase> &&event) noexcept;
    virtual void AnimationDidFinish() noexcept;
    KRYS_NODISCARD WebAnimationTime ZeroTime() const noexcept;

  private:
    void TimingDidChange(DidSeek didSeek, SynchronouslyNotify synchronouslyNotify,
                         Silently silently = Silently::No) noexcept;
    /// @see https://drafts.csswg.org/web-animations-1/#updating-the-finished-state
    void UpdateFinishedState(DidSeek didSeek, SynchronouslyNotify synchronouslyNotify) noexcept;
    KRYS_NODISCARD WebAnimationTime EffectEndTime() const noexcept;
    KRYS_NODISCARD Maybe<WebAnimationTime>
      CurrentTime(RespectHoldTime respectHoldTime,
                  UseCachedCurrentTime useCachedCurrentTime = UseCachedCurrentTime::Yes) const noexcept;
    /// @see https://drafts.csswg.org/web-animations-2/#silently-set-the-current-time
    dom::ExceptionOr<void> SilentlySetCurrentTime(Maybe<WebAnimationTime> seekTime) noexcept;
    /// @see https://drafts.csswg.org/web-animations-1/#finish-notification-steps
    void FinishNotificationSteps() noexcept;
    KRYS_NODISCARD bool HasPendingPauseTask() const noexcept
    {
      return _timeToRunPendingPauseTask != TimeToRunPendingTask::NotScheduled;
    }
    KRYS_NODISCARD bool HasPendingPlayTask() const noexcept
    {
      return _timeToRunPendingPlayTask != TimeToRunPendingTask::NotScheduled;
    }
    dom::ExceptionOr<void> Play(AutoRewind autoRewind) noexcept;
    void RunPendingPauseTask() noexcept;
    void RunPendingPlayTask() noexcept;
    /// @see https://drafts.csswg.org/web-animations-1/#reset-an-animations-pending-tasks
    void ResetPendingTasks() noexcept;
    void SetEffectInternal(RefPtr<AnimationEffect> &&newEffect,
                           bool doNotRemoveFromTimeline = false) noexcept;
    void SetTimelineInternal(RefPtr<AnimationTimeline> &&timeline) noexcept;
    KRYS_NODISCARD bool ComputeRelevance() noexcept;
    void InvalidateEffect() noexcept;
    /// @see https://drafts.csswg.org/web-animations/#effective-playback-rate
    KRYS_NODISCARD double EffectivePlaybackRate() const noexcept;
    /// @see https://drafts.csswg.org/web-animations/#apply-any-pending-playback-rate
    void ApplyPendingPlaybackRate() noexcept;
    /// @see https://drafts.csswg.org/web-animations-2/#ready
    void MaybeMarkAsReady() noexcept;
    /// @see https://drafts.csswg.org/web-animations-2/#validating-a-css-numberish-time
    KRYS_NODISCARD bool IsTimeValid(const Maybe<WebAnimationTime> &time) const noexcept;
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_WEB_ANIMATION(ToValueTypeName, predicate)                                \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(ToValueTypeName)                                                    \
    KRYS_NODISCARD static bool IsType(const krys::boo::WebAnimation &value) noexcept                         \
    {                                                                                                        \
      return value.predicate;                                                                                \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

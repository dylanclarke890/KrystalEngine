#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Animation/AnimationEffect.hpp"
#include "Krystal.Booey/Animation/AnimationPlaybackEvent.hpp"
#include "Krystal.Booey/Animation/AnimationTimeline.hpp"
#include "Krystal.Booey/Animation/DocumentTimeline.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/StyleOriginatedAnimation.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include <cmath>

namespace krys::boo
{
  Ref<WebAnimation> WebAnimation::Create(dom::Document &document, AnimationEffect *effect) noexcept
  {
    Ref<WebAnimation> result = AdoptRef(*new WebAnimation(document));
    result->SetEffect(ShareRefPtr(effect));
    result->SetTimeline(ShareRefPtr<AnimationTimeline>(&document.Timeline()));
    return result;
  }

  Ref<WebAnimation> WebAnimation::Create(dom::Document &document, AnimationEffect *effect,
                                         AnimationTimeline *timeline) noexcept
  {
    Ref<WebAnimation> result = AdoptRef(*new WebAnimation(document));
    result->SetEffect(ShareRefPtr(effect));
    if (timeline)
    {
      result->SetTimeline(ShareRefPtr(timeline));
    }
    else
    {
      AnimationTimeline::UpdateGlobalPosition(*result);
    }
    return result;
  }

  WebAnimation::WebAnimation(dom::Document &document) noexcept : _document(CreateWeakPtr(&document))
  {
  }

  WebAnimation::~WebAnimation() noexcept = default;

  void WebAnimation::Remove() noexcept
  {
    // This object could be deleted after either clearing the effect or timeline relationship.
    Ref protectedThis = ShareRef(*this);
    SetEffectInternal(nullptr);
    SetTimelineInternal(nullptr);
    _holdTime = null;
    _startTime = null;
  }

  void WebAnimation::SuspendEffectInvalidation() noexcept
  {
    ++_suspendCount;
  }

  void WebAnimation::UnsuspendEffectInvalidation() noexcept
  {
    krys_debug_assert(_suspendCount > 0);
    --_suspendCount;
  }

  void WebAnimation::EffectTimingDidChange() noexcept
  {
    TimingDidChange(DidSeek::No, SynchronouslyNotify::Yes);

    if (_effect)
    {
      _effect->AnimationDidChangeTimingProperties();
    }
  }

  void WebAnimation::SetId(dom::DOMString &&id) noexcept
  {
    _id = krys::move(id);
  }

  void WebAnimation::SetEffect(RefPtr<AnimationEffect> &&newEffect) noexcept
  {
    // 3.4.3. Setting the target effect of an animation
    // https://drafts.csswg.org/web-animations-1/#setting-the-target-effect

    // 1. Let old effect be the current target effect of animation, if any.
    auto oldEffect = _effect;

    // 2. If new effect is the same object as old effect, abort this procedure.
    if (newEffect == oldEffect)
    {
      return;
    }

    // 3. If animation has a pending pause task, reschedule that task to run as soon as animation is ready.
    if (HasPendingPauseTask())
    {
      _timeToRunPendingPauseTask = TimeToRunPendingTask::WhenReady;
    }

    // 4. If animation has a pending play task, reschedule that task to run as soon as animation is ready to
    // play
    //    new effect.
    if (HasPendingPlayTask())
    {
      _timeToRunPendingPlayTask = TimeToRunPendingTask::WhenReady;
    }

    // 5. If new effect is not null and if new effect is the target effect of another animation, previous
    //    animation, run the procedure to set the target effect of an animation (this procedure) on previous
    //    animation passing null as new effect.
    if (newEffect && newEffect->Animation())
    {
      newEffect->Animation()->SetEffect(nullptr);
    }

    // 6. Let the target effect of animation be new effect.
    // In the case of a style-originated animation, we don't want to remove the animation from the relevant
    // maps because while the effect was set via the API, the element still has a transition or animation set
    // up and we must not break the timeline-to-animation relationship.

    InvalidateEffect();

    // This object could be deleted after clearing the effect relationship.
    Ref protectedThis = ShareRef(*this);
    SetEffectInternal(krys::move(newEffect), IsStyleOriginatedAnimation());

    // 7. Run the procedure to update an animation's finished state for animation with the did seek flag set
    // to
    //    false, and the synchronously notify flag set to false.
    TimingDidChange(DidSeek::No, SynchronouslyNotify::No);

    InvalidateEffect();
  }

  void WebAnimation::SetEffectInternal(RefPtr<AnimationEffect> &&newEffect,
                                       bool doNotRemoveFromTimeline) noexcept
  {
    if (_effect == newEffect)
    {
      return;
    }

    RefPtr oldEffect = std::exchange(_effect, krys::move(newEffect));

    auto *oldKeyframeEffect = DynamicDowncast<KeyframeEffect>(oldEffect.get());
    Maybe<style::Styleable> previousTarget = oldKeyframeEffect ? oldKeyframeEffect->TargetStyleable() : null;

    auto *keyframeEffect = DynamicDowncast<KeyframeEffect>(_effect.get());
    Maybe<style::Styleable> newTarget = keyframeEffect ? keyframeEffect->TargetStyleable() : null;

    // Update the effect-to-animation relationships and the timeline's animation map.
    if (oldEffect)
    {
      oldEffect->SetAnimation(nullptr);
      if (!doNotRemoveFromTimeline && previousTarget && previousTarget != newTarget)
      {
        previousTarget->AnimationWasRemoved(*this);
      }
    }

    if (_effect)
    {
      _effect->SetAnimation(this);
      if (newTarget && previousTarget != newTarget)
      {
        newTarget->AnimationWasAdded(*this);
      }
    }
  }

  KeyframeEffect *WebAnimation::GetKeyframeEffect() const noexcept
  {
    return DynamicDowncast<KeyframeEffect>(_effect.get());
  }

  void WebAnimation::SetTimeline(RefPtr<AnimationTimeline> &&timeline) noexcept
  {
    // 3.4.1. Setting the timeline of an animation
    // https://drafts.csswg.org/web-animations-1/#setting-the-timeline

    // 1. Let old timeline be the current timeline of animation, if any.
    RefPtr oldTimeline = _timeline;

    // 2. If new timeline is the same object as old timeline, abort this procedure.
    if (timeline == oldTimeline)
    {
      return;
    }

    // 3. Let previous play state be animation's play state.
    auto previousPlayState = GetPlayState();

    // 4. Let previous current time be the animation's current time.
    auto previousCurrentTime = CurrentTime();

    // 5. Set previous progress based in the first condition that applies:
    auto previousProgress = [&]() -> Maybe<double>
    {
      // If previous current time is unresolved: Set previous progress to unresolved.
      if (!previousCurrentTime)
      {
        return null;
      }
      // If end time is zero: Set previous progress to zero.
      auto endTime = EffectEndTime();
      if (endTime.IsZero())
      {
        return 0.0;
      }
      // Otherwise: Set previous progress = previous current time / end time
      return *previousCurrentTime / endTime;
    }();

    // 6. Let from finite timeline be true if old timeline is not null and not monotonically increasing.
    auto fromFiniteTimeline = oldTimeline && !oldTimeline->IsMonotonic();

    // 7. Let to finite timeline be true if timeline is not null and not monotonically increasing.
    auto toFiniteTimeline = timeline && !timeline->IsMonotonic();

    // 8. Let the timeline of animation be new timeline.

    if (RefPtr keyframeEffect = ShareRefPtr(GetKeyframeEffect()))
    {
      if (auto target = keyframeEffect->TargetStyleable())
      {
        // In the case of a style-originated animation, we don't want to remove the animation from the
        // relevant maps because while the timeline was set via the API, the element still has a transition or
        // animation set up and we must not break the relationship.
        if (!IsStyleOriginatedAnimation())
        {
          target->AnimationWasRemoved(*this);
        }
        target->AnimationWasAdded(*this);
      }
    }

    // This object could be deleted after clearing the timeline relationship.
    Ref protectedThis = ShareRef(*this);
    SetTimelineInternal(krys::move(timeline));

    auto *documentTimeline = DynamicDowncast<DocumentTimeline>(_timeline.get());
    SetSuspended(documentTimeline && documentTimeline->AnimationsAreSuspended());

    // 9. Perform the steps corresponding to the first matching condition from the following, if any:
    if (toFiniteTimeline)
    {
      // If to finite timeline,
      // 1. Apply any pending playback rate on animation.
      ApplyPendingPlaybackRate();
      // 2. Set auto align start time to true.
      _autoAlignStartTime = true;
      // 3. Set start time to unresolved.
      _startTime = null;
      // 4. Set hold time to unresolved.
      _holdTime = null;
      if (previousPlayState == PlayState::Finished || previousPlayState == PlayState::Running)
      {
        // 5. If previous play state is "finished" or "running":
        //    Schedule a pending play task.
        _timeToRunPendingPlayTask = TimeToRunPendingTask::WhenReady;
      }
      else if (previousPlayState == PlayState::Paused && previousProgress)
      {
        // 6. If previous play state is "paused" and previous progress is resolved:
        //    Set hold time to previous progress * end time.
        _holdTime = EffectEndTime() * *previousProgress;
      }
    }
    else if (fromFiniteTimeline && previousProgress)
    {
      // If from finite timeline and previous progress is resolved,
      // Run the procedure to set the current time to previous progress * end time.
      (void)SetCurrentTime(EffectEndTime() * *previousProgress);
    }

    // 10. If the start time of animation is resolved, make animation's hold time unresolved.
    if (_startTime)
    {
      // FIXME: we may now be in a state where the hold time and start times have incompatible time units per
      // https://github.com/w3c/csswg-drafts/issues/11761. Until the spec knows how to handle this case, we
      // ensure the start time matches the value type of the currently resolved hold time before we make it
      // unresolved.
      if (_holdTime && _holdTime->Time().has_value() != _startTime->Time().has_value())
      {
        _startTime = _holdTime;
      }
      _holdTime = null;
    }

    // 11. Run the procedure to update an animation's finished state for animation with the did seek flag set
    // to
    //     false, and the synchronously notify flag set to false.
    TimingDidChange(DidSeek::No, SynchronouslyNotify::No);

    InvalidateEffect();
  }

  void WebAnimation::SetTimelineInternal(RefPtr<AnimationTimeline> &&timeline) noexcept
  {
    if (_timeline == timeline)
    {
      return;
    }

    if (_timeline)
    {
      _timeline->RemoveAnimation(*this);
    }

    _timeline = krys::move(timeline);

    if (_effect)
    {
      _effect->AnimationTimelineDidChange();
    }

    _pendingStartTime = null;
  }

  void WebAnimation::EffectTargetDidChange(const Maybe<style::Styleable> &previousTarget,
                                           const Maybe<style::Styleable> &newTarget) noexcept
  {
    if (_timeline)
    {
      if (previousTarget)
      {
        previousTarget->AnimationWasRemoved(*this);
      }

      if (newTarget)
      {
        newTarget->AnimationWasAdded(*this);
      }

      // This could have changed whether we have replaced animations, so we may need to schedule an update.
      _timeline->AnimationTimingDidChange(*this);
    }
  }

  void WebAnimation::SetStartTime(Maybe<WebAnimationTime> newStartTime) noexcept
  {
    // https://drafts.csswg.org/web-animations-2/#setting-the-start-time-of-an-animation

    // 1. Let valid start time be the result of running the validate a CSSNumberish time procedure with new
    // start
    //    time as the input.
    // 2. If valid start time is false, abort this procedure.
    // FIXME: Validating the time, once there's the script interface.

    // 3. Set auto align start time to false.
    _autoAlignStartTime = false;

    // 4. Let timeline time be the current time value of the timeline that animation is associated with. If
    // there
    //    is no timeline associated with animation or the associated timeline is inactive, let the timeline
    //    time be unresolved.
    auto timelineTime = _timeline ? _timeline->CurrentTime() : null;

    // 5. If timeline time is unresolved and new start time is resolved, make animation's hold time
    // unresolved.
    if (!timelineTime && newStartTime)
    {
      _holdTime = null;
    }

    // 6. Let previous current time be animation's current time.
    auto previousCurrentTime = CurrentTime();

    // 7. Apply any pending playback rate on animation.
    ApplyPendingPlaybackRate();

    // 8. Set animation's start time to new start time.
    _startTime = newStartTime;

    // 9. Update animation's hold time based on the first matching condition from the following,
    if (newStartTime)
    {
      // If new start time is resolved,
      // If animation's playback rate is not zero, make animation's hold time unresolved.
      if (_playbackRate)
      {
        _holdTime = null;
      }
    }
    else
    {
      // Otherwise (new start time is unresolved),
      // Set animation's hold time to previous current time even if previous current time is unresolved.
      _holdTime = previousCurrentTime;
    }

    // 10. If animation has a pending play task or a pending pause task, cancel that task and resolve
    // animation's
    //     current ready promise with animation.
    if (Pending())
    {
      _timeToRunPendingPauseTask = TimeToRunPendingTask::NotScheduled;
      _timeToRunPendingPlayTask = TimeToRunPendingTask::NotScheduled;
    }

    // 11. Run the procedure to update an animation's finished state for animation with the did seek flag set
    // to
    //     true, and the synchronously notify flag set to false.
    TimingDidChange(DidSeek::Yes, SynchronouslyNotify::No);

    InvalidateEffect();
  }

  Maybe<WebAnimationTime> WebAnimation::CurrentTime(UseCachedCurrentTime useCachedCurrentTime) const noexcept
  {
    return CurrentTime(RespectHoldTime::Yes, useCachedCurrentTime);
  }

  Maybe<WebAnimationTime> WebAnimation::CurrentTime(RespectHoldTime respectHoldTime,
                                                    UseCachedCurrentTime useCachedCurrentTime) const noexcept
  {
    // 3.4.4. The current time of an animation
    // https://drafts.csswg.org/web-animations-1/#the-current-time-of-an-animation

    // The current time is calculated from the first matching condition from below:

    // If the animation's hold time is resolved, the current time is the animation's hold time.
    if (respectHoldTime == RespectHoldTime::Yes && _holdTime)
    {
      return _holdTime;
    }

    // If any of the following are true:
    //     1. the animation has no associated timeline, or
    //     2. the associated timeline is inactive, or
    //     3. the animation's start time is unresolved.
    // The current time is an unresolved time value.
    if (!_timeline || !_startTime)
    {
      return null;
    }
    auto timelineTime = _timeline->CurrentTime(useCachedCurrentTime);
    if (!timelineTime)
    {
      return null;
    }

    // Otherwise, current time = (timeline time - start time) * playback rate
    return (*timelineTime - *_startTime) * _playbackRate;
  }

  dom::ExceptionOr<void> WebAnimation::SilentlySetCurrentTime(Maybe<WebAnimationTime> seekTime) noexcept
  {
    // https://drafts.csswg.org/web-animations-2/#silently-set-the-current-time

    // 1. If seek time is an unresolved time value, then perform the following steps.
    if (!seekTime)
    {
      // 1. If the current time is resolved, then throw a TypeError.
      if (CurrentTime())
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      // 2. Abort these steps.
      return {};
    }

    // 2. Let valid seek time be the result of running the validate a CSSNumberish time procedure with seek
    // time
    //    as the input.
    // 3. If valid seek time is false, abort this procedure.
    // FIXME: Validating the time, once there's the script interface.

    // 4. Set auto align start time to false.
    _autoAlignStartTime = false;

    // 5. Update either animation's hold time or start time as follows:
    // If any of the following conditions are true:
    //     - animation's hold time is resolved, or
    //     - animation's start time is unresolved, or
    //     - animation has no associated timeline or the associated timeline is inactive, or
    //     - animation's playback rate is 0,
    // Set animation's hold time to seek time.
    // Otherwise, set animation's start time to the result of evaluating timeline time - (seek time / playback
    // rate) where timeline time is the current time value of timeline associated with animation.
    if (_holdTime || !_startTime || !_timeline || !_timeline->CurrentTime() || !_playbackRate)
    {
      _holdTime = seekTime;
    }
    else
    {
      _startTime = *_timeline->CurrentTime() - (*seekTime / _playbackRate);
    }

    // 6. If animation has no associated timeline or the associated timeline is inactive, make animation's
    // start
    //    time unresolved.
    if (!_timeline || !_timeline->CurrentTime())
    {
      _startTime = null;
    }

    // 7. Make animation's previous current time unresolved.
    _previousCurrentTime = null;

    return {};
  }

  dom::ExceptionOr<void> WebAnimation::SetCurrentTime(Maybe<WebAnimationTime> seekTime) noexcept
  {
    // Setting the current time of an animation
    // https://drafts.csswg.org/web-animations-2/#setting-the-current-time-of-an-animation

    // 1. Run the steps to silently set the current time of animation to seek time.
    auto silentResult = SilentlySetCurrentTime(seekTime);
    if (silentResult.HasException())
    {
      return silentResult.ReleaseException();
    }

    // 2. If animation has a pending pause task, synchronously complete the pause operation by performing the
    //    following steps:
    if (HasPendingPauseTask())
    {
      // 1. Set animation's hold time to seek time.
      _holdTime = seekTime;
      // 2. Apply any pending playback rate to animation.
      ApplyPendingPlaybackRate();
      // 3. Make animation's start time unresolved.
      _startTime = null;
      // 4. Cancel the pending pause task.
      _timeToRunPendingPauseTask = TimeToRunPendingTask::NotScheduled;
      // 5. Resolve animation's current ready promise with animation.
    }

    // 3. Run the procedure to update an animation's finished state for animation with the did seek flag set
    // to
    //    true, and the synchronously notify flag set to false.
    TimingDidChange(DidSeek::Yes, SynchronouslyNotify::No);

    if (_effect)
    {
      _effect->AnimationDidChangeTimingProperties();
    }

    InvalidateEffect();

    return {};
  }

  double WebAnimation::EffectivePlaybackRate() const noexcept
  {
    // https://drafts.csswg.org/web-animations/#effective-playback-rate
    // The effective playback rate of an animation is its pending playback rate, if set, otherwise it is the
    // animation's playback rate.
    return _pendingPlaybackRate ? *_pendingPlaybackRate : _playbackRate;
  }

  void WebAnimation::SetPlaybackRate(double newPlaybackRate) noexcept
  {
    // Setting the playback rate of an animation
    // https://drafts.csswg.org/web-animations-1/#setting-the-playback-rate-of-an-animation

    // 1. Clear any pending playback rate on animation.
    _pendingPlaybackRate = null;

    // 2. Let previous time be the value of the current time of animation before changing the playback rate.
    auto previousTime = CurrentTime();

    // 3. Let previous playback rate be the current effective playback rate of animation.
    auto previousPlaybackRate = EffectivePlaybackRate();

    // 4. Set the playback rate to new playback rate.
    _playbackRate = newPlaybackRate;

    // 5. Perform the steps corresponding to the first matching condition from the following, if any:
    if (_timeline && _timeline->IsMonotonic() && previousTime)
    {
      // If animation is associated with a monotonically increasing timeline and the previous time is
      // resolved, Set the current time of animation to previous time.
      (void)SetCurrentTime(previousTime);
    }
    else if (_timeline && !_timeline->IsMonotonic() && _startTime && !EffectEndTime().IsInfinity()
             && ((previousPlaybackRate < 0 && _playbackRate >= 0)
                 || (previousPlaybackRate >= 0 && _playbackRate < 0)))
    {
      // If animation is associated with a non-null timeline that is not monotonically increasing, the start
      // time of animation is resolved, associated effect end is not infinity, and either:
      // - the previous playback rate < 0 and the new playback rate ≥ 0, or
      // - the previous playback rate ≥ 0 and the new playback rate < 0,
      // Set animation's start time to the result of evaluating associated effect end − start time for
      // animation.
      _startTime = EffectEndTime() - *_startTime;
    }

    if (_effect)
    {
      _effect->AnimationDidChangeTimingProperties();
      _effect->AnimationPlaybackRateDidChange();
    }
  }

  void WebAnimation::UpdatePlaybackRate(double newPlaybackRate) noexcept
  {
    // https://drafts.csswg.org/web-animations/#seamlessly-update-the-playback-rate

    // The procedure to seamlessly update the playback rate an animation, animation, to new playback rate
    // preserving its current time is as follows:

    // 1. Let previous play state be animation's play state.
    //    Note: It is necessary to record the play state before updating animation's effective playback rate
    //    since, in the following logic, we want to immediately apply the pending playback rate of animation
    //    if it is currently finished regardless of whether or not it will still be finished after we apply
    //    the pending playback rate.
    auto previousPlayState = GetPlayState();

    // 2. Let animation's pending playback rate be new playback rate.
    _pendingPlaybackRate = newPlaybackRate;

    // 3. Perform the steps corresponding to the first matching condition from below:
    if (Pending())
    {
      // If animation has a pending play task or a pending pause task,
      // Abort these steps.
      // Note: The different types of pending tasks will apply the pending playback rate when they run so
      // there is no further action required in this case.
      return;
    }

    if (previousPlayState == PlayState::Idle || previousPlayState == PlayState::Paused || !CurrentTime())
    {
      // If previous play state is idle or paused, or animation's current time is unresolved,
      // Apply any pending playback rate on animation.
      ApplyPendingPlaybackRate();
    }
    else if (previousPlayState == PlayState::Finished)
    {
      // If previous play state is finished,
      // 1. Let the unconstrained current time be the result of calculating the current time of animation
      //    substituting an unresolved time value for the hold time.
      auto unconstrainedCurrentTime = CurrentTime(RespectHoldTime::No);
      // 2. Let animation's start time be the result of evaluating the following expression:
      // timeline time - (unconstrained current time / pending playback rate)
      // Where timeline time is the current time value of the timeline associated with animation.
      // If pending playback rate is zero, let animation's start time be timeline time.
      // If timeline is inactive abort these steps.
      if (!_timeline->CurrentTime())
      {
        return;
      }
      auto newStartTime = *_timeline->CurrentTime();
      if (_pendingPlaybackRate && *_pendingPlaybackRate)
      {
        newStartTime -= (*unconstrainedCurrentTime / *_pendingPlaybackRate);
      }
      _startTime = newStartTime;
      // 3. Apply any pending playback rate on animation.
      ApplyPendingPlaybackRate();
      // 4. Run the procedure to update an animation's finished state for animation with the did seek flag set
      // to
      //    false, and the synchronously notify flag set to false.
      TimingDidChange(DidSeek::No, SynchronouslyNotify::No);

      InvalidateEffect();
    }
    else
    {
      // Otherwise,
      // Run the procedure to play an animation for animation with the auto-rewind flag set to false.
      (void)Play(AutoRewind::No);
    }

    if (_effect)
    {
      _effect->AnimationDidChangeTimingProperties();
    }
  }

  void WebAnimation::ApplyPendingPlaybackRate() noexcept
  {
    // https://drafts.csswg.org/web-animations/#apply-any-pending-playback-rate

    // 1. If animation does not have a pending playback rate, abort these steps.
    if (!_pendingPlaybackRate)
    {
      return;
    }

    // 2. Set animation's playback rate to its pending playback rate.
    _playbackRate = *_pendingPlaybackRate;

    // 3. Clear animation's pending playback rate.
    _pendingPlaybackRate = null;

    if (_effect)
    {
      _effect->AnimationPlaybackRateDidChange();
    }
  }

  WebAnimation::PlayState WebAnimation::GetPlayState() const noexcept
  {
    // 3.5.19 Play states
    // https://drafts.csswg.org/web-animations/#play-states

    // The play state of animation, animation, at a given moment is the state corresponding to the
    // first matching condition from the following:

    // The current time of animation is unresolved, and the start time of animation is unresolved, and
    // animation does not have either a pending play task or a pending pause task,
    // → idle
    auto animationCurrentTime = CurrentTime();
    if (!animationCurrentTime && !_startTime && !Pending())
    {
      return PlayState::Idle;
    }

    // Animation has a pending pause task, or both the start time of animation is unresolved and it does not
    // have a pending play task,
    // → paused
    if (HasPendingPauseTask() || (!_startTime && !HasPendingPlayTask()))
    {
      return PlayState::Paused;
    }

    // For animation, current time is resolved and either of the following conditions are true:
    // animation's effective playback rate > 0 and current time ≥ target effect end; or
    // animation's effective playback rate < 0 and current time ≤ 0,
    // → finished
    if (animationCurrentTime
        && ((EffectivePlaybackRate() > 0
             && (*animationCurrentTime + animationCurrentTime->MatchingEpsilon()) >= EffectEndTime())
            || (EffectivePlaybackRate() < 0
                && (*animationCurrentTime - animationCurrentTime->MatchingEpsilon())
                     <= animationCurrentTime->MatchingZero())))
    {
      return PlayState::Finished;
    }

    // Otherwise → running
    return PlayState::Running;
  }

  WebAnimationTime WebAnimation::ZeroTime() const noexcept
  {
    if ((_timeline && _timeline->IsProgressBased()) || (_startTime && _startTime->Percentage())
        || (_holdTime && _holdTime->Percentage()))
    {
      return WebAnimationTime::FromPercentage(0);
    }
    return Seconds {0};
  }

  WebAnimationTime WebAnimation::EffectEndTime() const noexcept
  {
    // The target effect end of an animation is equal to the end time of the animation's target effect.
    // If the animation has no target effect, the target effect end is zero.
    return _effect ? _effect->EndTime() : ZeroTime();
  }

  void WebAnimation::Cancel(Silently silently) noexcept
  {
    // 3.4.16. Canceling an animation
    // https://drafts.csswg.org/web-animations-1/#canceling-an-animation-section
    //
    // An animation can be canceled which causes the current time to become unresolved hence removing any
    // effects caused by the target effect.
    //
    // The procedure to cancel an animation for animation is as follows:
    //
    // 1. If animation's play state is not idle, perform the following steps:
    if (GetPlayState() != PlayState::Idle)
    {
      // 1. Run the procedure to reset an animation's pending tasks on animation.
      ResetPendingTasks();

      // 2. Reject the current finished promise with a DOMException named "AbortError".
      // 3. Set the [[PromiseIsHandled]] internal slot of the current finished promise to true.
      // 4. Let current finished promise be a new (pending) Promise object.
      _finishedPromiseIsFulfilled = false;

      // 5. Create an AnimationPlaybackEvent, cancelEvent.
      // 6. Set cancelEvent's type attribute to cancel.
      // 7. Set cancelEvent's currentTime to null.
      // 8. Let timeline time be the current time of the timeline with which animation is associated. If
      //    animation is not associated with an active timeline, let timeline time be n unresolved time value.
      // 9. Set cancelEvent's timelineTime to timeline time. If timeline time is unresolved, set it to null.
      // 10. If animation has a document for timing, then append cancelEvent to its document for timing's
      // pending
      //    animation event queue along with its target, animation. If animation is associated with an active
      //    timeline that defines a procedure to convert timeline times to origin-relative time, let the
      //    scheduled event time be the result of applying that procedure to timeline time. Otherwise, the
      //    scheduled event time is an unresolved time value.
      // Otherwise, queue a task to dispatch cancelEvent at animation. The task source for this task is the
      // DOM manipulation task source.
      auto scheduledTime = [&]() -> Maybe<WebAnimationTime>
      {
        if (auto *documentTimeline = DynamicDowncast<DocumentTimeline>(_timeline.get()))
        {
          if (auto currentTime = documentTimeline->CurrentTime())
          {
            if (auto time = currentTime->Time())
            {
              return WebAnimationTime {documentTimeline->ConvertTimelineTimeToOriginRelativeTime(*time)};
            }
          }
        }
        return null;
      }();
      EnqueueAnimationPlaybackEvent(EventNames::Cancel, null, scheduledTime);
    }

    // 2. Make animation's hold time unresolved.
    _holdTime = null;

    // 3. Make animation's start time unresolved.
    _startTime = null;

    TimingDidChange(DidSeek::No, SynchronouslyNotify::No, silently);

    InvalidateEffect();

    if (_effect)
    {
      _effect->AnimationWasCanceled();
    }
  }

  void WebAnimation::WillChangeRenderer() noexcept
  {
    if (RefPtr keyframeEffect = ShareRefPtr(GetKeyframeEffect()))
    {
      keyframeEffect->WillChangeRenderer();
    }
  }

  void WebAnimation::EnqueueAnimationPlaybackEvent(dom::DOMStringAtom type,
                                                   Maybe<WebAnimationTime> currentTime,
                                                   Maybe<WebAnimationTime> scheduledTime) noexcept
  {
    auto timelineTime = _timeline ? _timeline->CurrentTime() : null;
    Ref event = CreateRef<AnimationPlaybackEvent>(type, this, scheduledTime, timelineTime, currentTime);
    event->SetTarget(ShareRefPtr<dom::EventTarget>(this));
    EnqueueAnimationEvent(krys::move(event));
  }

  void WebAnimation::EnqueueAnimationEvent(Ref<AnimationEventBase> &&event) noexcept
  {
    // FIXME: The document timeline of a scroll timeline's source, once there are scroll timelines.
    auto documentTimeline = [&]() -> DocumentTimeline *
    {
      if (auto *timeline = DynamicDowncast<DocumentTimeline>(_timeline.get()))
      {
        return timeline;
      }
      if (RefPtr keyframeEffect = ShareRefPtr(GetKeyframeEffect()))
      {
        if (RefPtr target = ShareRefPtr(keyframeEffect->Target()))
        {
          return target->NodeDocument().ExistingTimeline();
        }
      }
      return nullptr;
    };

    if (RefPtr timeline = ShareRefPtr(documentTimeline()))
    {
      // If animation has a document for timing, then append event to its document for timing's pending
      // animation event queue along with its target, animation. If animation is associated with an active
      // timeline that defines a procedure to convert timeline times to origin-relative time, let the
      // scheduled event time be the result of applying that procedure to timeline time. Otherwise, the
      // scheduled event time is an unresolved time value.
      _hasScheduledEventsDuringTick = true;
      timeline->EnqueueAnimationEvent(*event);
    }
    else
    {
      // Otherwise, queue a task to dispatch event at animation. The task source for this task is the DOM
      // manipulation task source.
      html::EventLoop::Current().QueueTask(html::TaskSource::DOMManipulation,
                                           [event = krys::move(event)]
                                           {
                                             RefPtr target = event->Target();
                                             (void)dom::EventDispatcher::DispatchToTarget(*event, target.get());
                                           });
    }
  }

  void WebAnimation::AnimationDidFinish() noexcept
  {
    if (_effect)
    {
      _effect->AnimationDidFinish();
    }
  }

  void WebAnimation::ResetPendingTasks() noexcept
  {
    // The procedure to reset an animation's pending tasks for animation is as follows:
    // https://drafts.csswg.org/web-animations-1/#reset-an-animations-pending-tasks
    //
    // 1. If animation does not have a pending play task or a pending pause task, abort this procedure.
    if (!Pending())
    {
      return;
    }

    // 2. If animation has a pending play task, cancel that task.
    if (HasPendingPlayTask())
    {
      _timeToRunPendingPlayTask = TimeToRunPendingTask::NotScheduled;
    }

    // 3. If animation has a pending pause task, cancel that task.
    if (HasPendingPauseTask())
    {
      _timeToRunPendingPauseTask = TimeToRunPendingTask::NotScheduled;
    }

    // 4. Apply any pending playback rate on animation.
    ApplyPendingPlaybackRate();

    // 5. Reject animation's current ready promise with a DOMException named "AbortError".
    // 6. Set the [[PromiseIsHandled]] internal slot of animation’s current ready promise to true.
    // 7. Let animation's current ready promise be the result of creating a new resolved Promise object.
  }

  dom::ExceptionOr<void> WebAnimation::Finish() noexcept
  {
    // 3.4.15. Finishing an animation
    // https://drafts.csswg.org/web-animations-1/#finishing-an-animation-section

    // An animation can be advanced to the natural end of its current playback direction by using the
    // procedure to finish an animation for animation defined below:
    //
    // 1. If animation's effective playback rate is zero, or if animation's effective playback rate > 0 and
    // target
    //    effect end is infinity, throw an InvalidStateError and abort these steps.
    if (!EffectivePlaybackRate() || (EffectivePlaybackRate() > 0 && EffectEndTime().IsInfinity()))
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    // 2. Apply any pending playback rate to animation.
    ApplyPendingPlaybackRate();

    // 3. Set limit as follows:
    // If animation playback rate > 0, let limit be target effect end.
    // Otherwise, let limit be zero.
    auto limit = _playbackRate > 0 ? EffectEndTime() : ZeroTime();

    // 4. Silently set the current time to limit.
    (void)SilentlySetCurrentTime(limit);

    // 5. If animation's start time is unresolved and animation has an associated active timeline, let the
    // start
    //    time be the result of evaluating timeline time - (limit / playback rate) where timeline time is the
    //    current time value of the associated timeline.
    if (!_startTime && _timeline && _timeline->CurrentTime())
    {
      _startTime = *_timeline->CurrentTime() - (limit / _playbackRate);
    }

    // 6. If there is a pending pause task and start time is resolved,
    if (HasPendingPauseTask() && _startTime)
    {
      // 1. Let the hold time be unresolved.
      _holdTime = null;
      // 2. Cancel the pending pause task.
      _timeToRunPendingPauseTask = TimeToRunPendingTask::NotScheduled;
      // 3. Resolve the current ready promise of animation with animation.
    }

    // 7. If there is a pending play task and start time is resolved, cancel that task and resolve the current
    //    ready promise of animation with animation.
    if (HasPendingPlayTask() && _startTime)
    {
      _timeToRunPendingPlayTask = TimeToRunPendingTask::NotScheduled;
    }

    // 8. Run the procedure to update an animation's finished state animation with the did seek flag set to
    // true,
    //    and the synchronously notify flag set to true.
    TimingDidChange(DidSeek::Yes, SynchronouslyNotify::Yes);

    InvalidateEffect();

    return {};
  }

  void WebAnimation::TimingDidChange(DidSeek didSeek, SynchronouslyNotify synchronouslyNotify,
                                     Silently silently) noexcept
  {
    _shouldSkipUpdatingFinishedStateWhenResolving = false;
    UpdateFinishedState(didSeek, synchronouslyNotify);

    if (silently == Silently::No && _timeline)
    {
      _timeline->AnimationTimingDidChange(*this);
    }
  }

  void WebAnimation::InvalidateEffect() noexcept
  {
    if (IsEffectInvalidationSuspended())
    {
      return;
    }

    if (RefPtr keyframeEffect = ShareRefPtr(GetKeyframeEffect()))
    {
      keyframeEffect->Invalidate();
    }
  }

  void WebAnimation::UpdateFinishedState(DidSeek didSeek, SynchronouslyNotify synchronouslyNotify) noexcept
  {
    // 3.4.14. Updating the finished state
    // https://drafts.csswg.org/web-animations-1/#updating-the-finished-state

    // 1. Let the unconstrained current time be the result of calculating the current time substituting an
    //    unresolved time value for the hold time if did seek is false. If did seek is true, the unconstrained
    //    current time is equal to the current time.
    auto unconstrainedCurrentTime =
      CurrentTime(didSeek == DidSeek::Yes ? RespectHoldTime::Yes : RespectHoldTime::No);
    auto endTime = EffectEndTime();

    // 2. If all three of the following conditions are true,
    //    - the unconstrained current time is resolved, and
    //    - animation's start time is resolved, and
    //    - animation does not have a pending play task or a pending pause task,
    if (unconstrainedCurrentTime && _startTime && !Pending())
    {
      // then update animation's hold time based on the first matching condition for animation from below, if
      // any:
      if (_playbackRate > 0 && *unconstrainedCurrentTime >= endTime)
      {
        // If animation playback rate > 0 and unconstrained current time is greater than or equal to target
        // effect end, If did seek is true, let the hold time be the value of unconstrained current time.
        if (didSeek == DidSeek::Yes)
        {
          _holdTime = unconstrainedCurrentTime;
        }
        // If did seek is false, let the hold time be the maximum value of previous current time and target
        // effect end. If the previous current time is unresolved, let the hold time be target effect end.
        else if (!_previousCurrentTime)
        {
          _holdTime = endTime;
        }
        else
        {
          _holdTime = std::max(*_previousCurrentTime, endTime);
        }
      }
      else if (_playbackRate < 0 && *unconstrainedCurrentTime <= unconstrainedCurrentTime->MatchingZero())
      {
        // If animation playback rate < 0 and unconstrained current time is less than or equal to 0,
        // If did seek is true, let the hold time be the value of unconstrained current time.
        if (didSeek == DidSeek::Yes)
        {
          _holdTime = unconstrainedCurrentTime;
        }
        // If did seek is false, let the hold time be the minimum value of previous current time and zero. If
        // the previous current time is unresolved, let the hold time be zero.
        else if (!_previousCurrentTime)
        {
          _holdTime = ZeroTime();
        }
        else
        {
          _holdTime = std::min(*_previousCurrentTime, _previousCurrentTime->MatchingZero());
        }
      }
      else if (_playbackRate && _timeline && _timeline->CurrentTime())
      {
        // If animation playback rate ≠ 0, and animation is associated with an active timeline,
        // Perform the following steps:
        // 1. If did seek is true and the hold time is resolved, let animation's start time be equal to the
        // result
        //    of evaluating timeline time - (hold time / playback rate) where timeline time is the current
        //    time value of timeline associated with animation.
        if (didSeek == DidSeek::Yes && _holdTime)
        {
          _startTime = *_timeline->CurrentTime() - (*_holdTime / _playbackRate);
        }
        // 2. Let the hold time be unresolved.
        _holdTime = null;
      }
    }

    // 3. Set the previous current time of animation be the result of calculating its current time.
    _previousCurrentTime = CurrentTime();

    // 4. Let current finished state be true if the play state of animation is finished. Otherwise, let it be
    //    false.
    auto currentFinishedState = GetPlayState() == PlayState::Finished;

    // 5. If current finished state is true and the current finished promise is not yet resolved, perform the
    //    following steps:
    if (currentFinishedState && !_finishedPromiseIsFulfilled)
    {
      AnimationDidFinish();
      if (synchronouslyNotify == SynchronouslyNotify::Yes)
      {
        // If synchronously notify is true, cancel any queued microtask to run the finish notification steps
        // for this animation, and run the finish notification steps immediately.
        _finishNotificationStepsMicrotaskPending = false;
        FinishNotificationSteps();
      }
      else if (!_finishNotificationStepsMicrotaskPending)
      {
        // Otherwise, if synchronously notify is false, queue a microtask to run finish notification steps for
        // animation unless there is already a microtask queued to run those steps for animation.
        _finishNotificationStepsMicrotaskPending = true;
        html::EventLoop::Current().QueueMicrotask(
          [this, protectedThis = ShareRef(*this)]
          {
            if (_finishNotificationStepsMicrotaskPending)
            {
              _finishNotificationStepsMicrotaskPending = false;
              FinishNotificationSteps();
            }
          });
      }
    }

    // 6. If current finished state is false and animation's current finished promise is already resolved, set
    //    animation's current finished promise to a new (pending) Promise object.
    if (!currentFinishedState && _finishedPromiseIsFulfilled)
    {
      _finishedPromiseIsFulfilled = false;
    }

    UpdateRelevance();
  }

  void WebAnimation::FinishNotificationSteps() noexcept
  {
    // 3.4.14. Updating the finished state
    // https://drafts.csswg.org/web-animations-1/#finish-notification-steps

    // Let finish notification steps refer to the following procedure:
    // 1. If animation's play state is not equal to finished, abort these steps.
    if (GetPlayState() != PlayState::Finished)
    {
      return;
    }

    // 2. Resolve animation's current finished promise object with animation.
    _finishedPromiseIsFulfilled = true;

    // 3. Create an AnimationPlaybackEvent, finishEvent.
    // 4. Set finishEvent's type attribute to finish.
    // 5. Set finishEvent's currentTime attribute to the current time of animation.
    // 6. Set finishEvent's timelineTime attribute to the current time of the timeline with which animation is
    //    associated. If animation is not associated with a timeline, or the timeline is inactive, let
    //    timelineTime be null.
    // 7. If animation has a document for timing, then append finishEvent to its document for timing's pending
    //    animation event queue along with its target, animation. For the scheduled event time, use the result
    //    of converting animation's target effect end to an origin-relative time. Otherwise, queue a task to
    //    dispatch finishEvent at animation. The task source for this task is the DOM manipulation task
    //    source.
    auto scheduledTime = [&]() -> Maybe<WebAnimationTime>
    {
      if (auto *documentTimeline = DynamicDowncast<DocumentTimeline>(_timeline.get()))
      {
        if (auto effectEndTime = EffectEndTime().Time())
        {
          if (auto animationEndTime = ConvertAnimationTimeToTimelineTime(*effectEndTime))
          {
            return WebAnimationTime {
              documentTimeline->ConvertTimelineTimeToOriginRelativeTime(*animationEndTime)};
          }
        }
      }
      return null;
    }();
    EnqueueAnimationPlaybackEvent(EventNames::Finish, CurrentTime(), scheduledTime);
  }

  dom::ExceptionOr<void> WebAnimation::Play() noexcept
  {
    return Play(AutoRewind::Yes);
  }

  dom::ExceptionOr<void> WebAnimation::Play(AutoRewind autoRewind) noexcept
  {
    // Playing an animation
    // https://drafts.csswg.org/web-animations-2/#playing-an-animation-section

    auto playbackRate = EffectivePlaybackRate();
    auto endTime = EffectEndTime();

    // 1. Let aborted pause be a boolean flag that is true if animation has a pending pause task, and false
    //    otherwise.
    bool abortedPause = HasPendingPauseTask();

    // 2. Let has pending ready promise be a boolean flag that is initially false.

    // 3. Let has finite timeline be true if animation has an associated timeline that is not monotonically
    //    increasing.
    auto hasFiniteTimeline = _timeline && !_timeline->IsMonotonic();

    // 4. Let previous current time be the animation’s current time
    auto previousCurrentTime = CurrentTime();

    // 5. Let enable seek be true if the auto-rewind flag is true and has finite timeline is false. Otherwise,
    //    initialize to false.
    auto enableSeek = autoRewind == AutoRewind::Yes && !hasFiniteTimeline;

    // 6. Perform the steps corresponding to the first matching condition from the following, if any::
    if ((playbackRate > 0 && enableSeek)
        && (!previousCurrentTime || *previousCurrentTime < Seconds {0}
            || (*previousCurrentTime + previousCurrentTime->MatchingEpsilon()) >= endTime))
    {
      // If animation’s effective playback rate > 0, enable seek is true and either animation’s:
      //     - previous current time is unresolved, or
      //     - previous current time < zero, or
      //     - previous current time ≥ associated effect end,
      // Set the animation’s hold time to zero.
      _holdTime = Seconds {0};
    }
    else if ((playbackRate < 0 && enableSeek)
             && (!previousCurrentTime || *previousCurrentTime <= Seconds {0}
                 || *previousCurrentTime > endTime))
    {
      // If animation’s effective playback rate < 0, enable seek is true and either animation’s:
      //     - previous current time is unresolved, or
      //     - previous current time ≤ zero, or
      //     - previous current time > associated effect end,
      // If associated effect end is positive infinity,
      //     throw an "InvalidStateError" DOMException and abort these steps.
      // Otherwise,
      //     Set the animation’s hold time to the animation’s associated effect end.
      if (endTime.IsInfinity())
      {
        return dom::Exception {dom::ExceptionCode::InvalidStateError};
      }
      _holdTime = endTime;
    }
    else if (!playbackRate && !previousCurrentTime)
    {
      // If animation’s effective playback rate = 0 and animation’s current time is unresolved,
      // Set the animation’s hold time to zero.
      _holdTime = ZeroTime();
    }

    // 7. If has finite timeline and previous current time is unresolved:
    // Set the flag auto align start time to true.
    if (hasFiniteTimeline && !previousCurrentTime)
    {
      _autoAlignStartTime = true;
    }

    // 8. If animation’s hold time is resolved, let its start time be unresolved.
    if (_holdTime)
    {
      _startTime = null;
    }

    // 9. If animation has a pending play task or a pending pause task,
    //     - Cancel that task.
    //     - Set has pending ready promise to true.
    if (Pending())
    {
      _timeToRunPendingPauseTask = TimeToRunPendingTask::NotScheduled;
      _timeToRunPendingPlayTask = TimeToRunPendingTask::NotScheduled;
    }

    // 10. If the following three conditions are all satisfied:
    //    - animation's hold time is unresolved, and
    //    - aborted pause is false, and
    //    - animation does not have a pending playback rate,
    // abort this procedure.
    // FIXME: the spec does not require the computation of pendingAutoAlignedStartTime and accounting for it,
    // but without it we never schedule a pending play task for scroll-driven animations.
    auto pendingAutoAlignedStartTime = _autoAlignStartTime && !_startTime;
    if (!_holdTime && !abortedPause && !_pendingPlaybackRate && !pendingAutoAlignedStartTime)
    {
      return {};
    }

    // 11. If has pending ready promise is false, let animation's current ready promise be a new promise in
    // the
    //     relevant Realm of animation.

    // 12. Schedule a task to run as soon as animation is ready.
    _timeToRunPendingPlayTask = TimeToRunPendingTask::WhenReady;

    // 13. Run the procedure to update an animation's finished state for animation with the did seek flag set
    // to
    //     false, and the synchronously notify flag set to false.
    TimingDidChange(DidSeek::No, SynchronouslyNotify::No);

    InvalidateEffect();

    return {};
  }

  void WebAnimation::RunPendingPlayTask() noexcept
  {
    // Playing an animation, step 12.
    // https://drafts.csswg.org/web-animations-2/#playing-an-animation-section

    _timeToRunPendingPlayTask = TimeToRunPendingTask::NotScheduled;

    // 1. Assert that at least one of animation’s start time or hold time is resolved.
    krys_debug_assert(_startTime || _holdTime);

    // 2. Let ready time be the time value of the timeline associated with animation at the moment when
    // animation
    //    became ready.
    auto readyTime = _pendingStartTime;
    if (!readyTime)
    {
      readyTime = _timeline->CurrentTime();
    }

    // 3. Perform the steps corresponding to the first matching condition below, if any:
    if (_holdTime)
    {
      // If animation's hold time is resolved,
      // 1. Apply any pending playback rate on animation.
      ApplyPendingPlaybackRate();
      // 2. Let new start time be the result of evaluating ready time - hold time / animation playback rate
      // for
      //    animation. If the animation playback rate is zero, let new start time be simply ready time.
      // FIXME: Implementation cannot guarantee an active timeline at the point of this async dispatch.
      // Subsequently, the resulting readyTime value can be null. Avoid null dereferencing by defaulting to a
      // zero value.
      auto newStartTime = readyTime.value_or(Seconds {0});
      if (_playbackRate)
      {
        newStartTime -= *_holdTime / _playbackRate;
      }
      // 3. Set the start time of animation to new start time.
      _startTime = newStartTime;
      // 4. If animation's playback rate is not 0, make animation's hold time unresolved.
      if (_playbackRate)
      {
        _holdTime = null;
      }
    }
    else if (_startTime && _pendingPlaybackRate)
    {
      // If animation's start time is resolved and animation has a pending playback rate,
      // 1. Let current time to match be the result of evaluating (ready time - start time) × playback rate
      // for
      //    animation.
      auto currentTimeToMatch = (readyTime.value_or(Seconds {0}) - *_startTime) * _playbackRate;
      // 2. Apply any pending playback rate on animation.
      ApplyPendingPlaybackRate();
      // 3. If animation's playback rate is zero, let animation's hold time be current time to match.
      if (_playbackRate)
      {
        _holdTime = currentTimeToMatch;
      }
      // 4. Let new start time be the result of evaluating ready time - current time to match / playback rate
      // for
      //    animation. If the playback rate is zero, let new start time be simply ready time.
      auto newStartTime = readyTime.value_or(Seconds {0});
      if (_playbackRate)
      {
        newStartTime -= currentTimeToMatch / _playbackRate;
      }
      // 5. Set the start time of animation to new start time.
      _startTime = newStartTime;
    }

    // 4. Resolve animation's current ready promise with animation.

    // 5. Run the procedure to update an animation's finished state for animation with the did seek flag set
    // to
    //    false, and the synchronously notify flag set to false.
    TimingDidChange(DidSeek::No, SynchronouslyNotify::No, Silently::Yes);

    InvalidateEffect();
  }

  dom::ExceptionOr<void> WebAnimation::Pause() noexcept
  {
    // Pausing an animation
    // https://drafts.csswg.org/web-animations-2/#pausing-an-animation-section

    // 1. If animation has a pending pause task, abort these steps.
    if (HasPendingPauseTask())
    {
      return {};
    }

    // 2. If the play state of animation is paused, abort these steps.
    if (GetPlayState() == PlayState::Paused)
    {
      return {};
    }

    // 3. Let has finite timeline be true if animation has an associated timeline that is not monotonically
    //    increasing.
    auto hasFiniteTimeline = _timeline && !_timeline->IsMonotonic();

    auto localTime = CurrentTime();

    // 4. If the animation’s current time is unresolved and has finite timeline is false, perform the steps
    //    according to the first matching condition below:
    if (!localTime && !hasFiniteTimeline)
    {
      if (_playbackRate >= 0)
      {
        // If animation's playback rate is ≥ 0, let animation's hold time be zero.
        _holdTime = ZeroTime();
      }
      else if (EffectEndTime().IsInfinity())
      {
        // Otherwise, if target effect end for animation is positive infinity, throw an InvalidStateError and
        // abort these steps.
        return dom::Exception {dom::ExceptionCode::InvalidStateError};
      }
      else
      {
        // Otherwise, let animation's hold time be target effect end.
        _holdTime = EffectEndTime();
      }
    }

    // 5. If has finite timeline is true, and the animation’s current time is unresolved,
    // Set the auto align start time flag to true.
    if (hasFiniteTimeline && !localTime)
    {
      _autoAlignStartTime = true;
    }

    // 6. Let has pending ready promise be a boolean flag that is initially false.

    // 7. If animation has a pending play task, cancel that task and let has pending ready promise be true.
    if (HasPendingPlayTask())
    {
      _timeToRunPendingPlayTask = TimeToRunPendingTask::NotScheduled;
    }

    // 8. If has pending ready promise is false, set animation's current ready promise to a new (pending)
    // Promise
    //    object.

    // 9. Schedule a task to be executed at the first possible moment where all of the following conditions
    // are
    //    true:
    //     - the user agent has performed any processing necessary to suspend the playback of animation’s
    //       associated effect, if any.
    //     - the animation is associated with a timeline that is not inactive.
    //     - the animation has a resolved hold time or start time.
    _timeToRunPendingPauseTask = TimeToRunPendingTask::ASAP;

    // 8. Run the procedure to update an animation's finished state for animation with the did seek flag set
    // to
    //    false, and the synchronously notify flag set to false.
    TimingDidChange(DidSeek::No, SynchronouslyNotify::No);

    InvalidateEffect();

    return {};
  }

  dom::ExceptionOr<void> WebAnimation::Reverse() noexcept
  {
    // 3.4.18. Reversing an animation
    // https://drafts.csswg.org/web-animations-1/#reverse-an-animation

    // The procedure to reverse an animation of animation animation is as follows:

    // 1. If there is no timeline associated with animation, or the associated timeline is inactive
    //    throw an InvalidStateError and abort these steps.
    if (!_timeline || !_timeline->CurrentTime())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    // 2. Let original pending playback rate be animation's pending playback rate.
    auto originalPendingPlaybackRate = _pendingPlaybackRate;

    // 3. Let animation's pending playback rate be the additive inverse of its effective playback rate (i.e.
    //    -effective playback rate).
    _pendingPlaybackRate = -EffectivePlaybackRate();

    // 4. Run the steps to play an animation for animation with the auto-rewind flag set to true.
    auto playResult = Play(AutoRewind::Yes);

    // If the steps to play an animation throw an exception, set animation's pending playback rate to original
    // pending playback rate and propagate the exception.
    if (playResult.HasException())
    {
      _pendingPlaybackRate = originalPendingPlaybackRate;
      return playResult.ReleaseException();
    }

    if (_effect)
    {
      _effect->AnimationDidChangeTimingProperties();
    }

    return {};
  }

  void WebAnimation::RunPendingPauseTask() noexcept
  {
    // 3.4.11. Pausing an animation, step 10.
    // https://drafts.csswg.org/web-animations-1/#pause-an-animation

    _timeToRunPendingPauseTask = TimeToRunPendingTask::NotScheduled;

    // 1. Let ready time be the time value of the timeline associated with animation at the moment when the
    // user
    //    agent completed processing necessary to suspend playback of animation's target effect.
    auto readyTime = _pendingStartTime;
    if (!readyTime)
    {
      readyTime = _timeline->CurrentTime();
    }

    auto animationStartTime = _startTime;

    // 2. If animation's start time is resolved and its hold time is not resolved, let animation's hold time
    // be
    //    the result of evaluating (ready time - start time) × playback rate.
    //    Note: The hold time might be already set if the animation is finished, or if the animation is
    //    pending, waiting to begin playback. In either case we want to preserve the hold time as we enter the
    //    paused state.
    if (animationStartTime && !_holdTime)
    {
      // FIXME: Implementation cannot guarantee an active timeline at the point of this async dispatch.
      // Subsequently, the resulting readyTime value can be null. Avoid null dereferencing by defaulting to a
      // zero value.
      _holdTime = (readyTime.value_or(Seconds {0}) - *animationStartTime) * _playbackRate;
    }

    // 3. Apply any pending playback rate on animation.
    ApplyPendingPlaybackRate();

    // 4. Make animation's start time unresolved.
    _startTime = null;

    // 5. Resolve animation's current ready promise with animation.

    // 6. Run the procedure to update an animation's finished state for animation with the did seek flag set
    // to
    //    false, and the synchronously notify flag set to false.
    TimingDidChange(DidSeek::No, SynchronouslyNotify::No, Silently::Yes);

    InvalidateEffect();
  }

  bool WebAnimation::NeedsTick() const noexcept
  {
    return Pending() || GetPlayState() == PlayState::Running || _hasScheduledEventsDuringTick;
  }

  void WebAnimation::Tick() noexcept
  {
    auto wasPending = Pending();

    // FIXME: Auto-aligning the start time with progress-based timelines, once there are scroll timelines.

    MaybeMarkAsReady();

    _hasScheduledEventsDuringTick = false;
    UpdateFinishedState(DidSeek::No, SynchronouslyNotify::Yes);
    _shouldSkipUpdatingFinishedStateWhenResolving = true;

    if (!IsEffectInvalidationSuspended() && _effect)
    {
      _effect->AnimationDidTick();
      if (wasPending && !Pending())
      {
        _effect->AnimationBecameReady();
      }
    }
  }

  void WebAnimation::MaybeMarkAsReady() noexcept
  {
    // https://drafts.csswg.org/web-animations-2/#ready
    // An animation is ready at the first moment where all of the following conditions are true:
    //     - the user agent has completed any setup required to begin the playback of each inclusive
    //     descendant
    //       of the animation’s associated effect including rendering the first frame of any keyframe effect
    //       or executing any custom effects associated with an animation effect.
    //     - the animation is associated with a timeline that is not inactive.
    //     - the animation’s hold time or start time is resolved.
    if (!Pending())
    {
      return;
    }

    auto isReady = _timeline && _timeline->CurrentTime() && (_holdTime || _startTime);
    if (!isReady)
    {
      return;
    }

    // Monotonic animations also require a pending start time.
    if (!_pendingStartTime && _timeline->IsMonotonic())
    {
      return;
    }

    // The effect can also prevent readines.
    if (_effect && _effect->PreventsAnimationReadiness())
    {
      return;
    }

    if (HasPendingPauseTask())
    {
      RunPendingPauseTask();
    }

    if (HasPendingPlayTask())
    {
      RunPendingPlayTask();
    }

    _pendingStartTime = null;
  }

  void WebAnimation::SetPendingStartTime(WebAnimationTime pendingStartTime) noexcept
  {
    _pendingStartTime = pendingStartTime;
    auto *documentTimeline = DynamicDowncast<DocumentTimeline>(_timeline.get());
    krys_debug_assert(documentTimeline);
    if (documentTimeline)
    {
      Ref protectedTimeline = ShareRef(*documentTimeline);
      protectedTimeline->PendingStartTimeWasSetOnAnimation();
    }
  }

  EnumSet<AnimationImpact>
    WebAnimation::Resolve(RenderStyle &targetStyle, const style::ResolutionContext &resolutionContext,
                          EndpointInclusiveActiveInterval endpointInclusiveActiveInterval) noexcept
  {
    if (!_shouldSkipUpdatingFinishedStateWhenResolving)
    {
      UpdateFinishedState(DidSeek::No, SynchronouslyNotify::No);
    }
    _shouldSkipUpdatingFinishedStateWhenResolving = false;

    if (RefPtr keyframeEffect = ShareRefPtr(GetKeyframeEffect()))
    {
      return keyframeEffect->Apply(targetStyle, resolutionContext, endpointInclusiveActiveInterval);
    }
    return {};
  }

  void WebAnimation::SetSuspended(bool isSuspended) noexcept
  {
    if (_isSuspended == isSuspended)
    {
      return;
    }

    _isSuspended = isSuspended;

    if (_effect && GetPlayState() == PlayState::Running)
    {
      _effect->AnimationSuspensionStateDidChange(isSuspended);
    }
  }

  void WebAnimation::UpdateRelevance() noexcept
  {
    auto wasRelevant = std::exchange(_isRelevant, ComputeRelevance());
    if (wasRelevant != _isRelevant)
    {
      if (RefPtr keyframeEffect = ShareRefPtr(GetKeyframeEffect()))
      {
        keyframeEffect->AnimationRelevancyDidChange();
      }
    }
  }

  bool WebAnimation::ComputeRelevance() noexcept
  {
    // https://drafts.csswg.org/web-animations-1/#relevant-animations-section
    // https://drafts.csswg.org/web-animations-1/#current
    // https://drafts.csswg.org/web-animations-1/#in-effect

    // An animation is relevant if:
    // - its associated effect is current or in effect, and
    if (!_effect)
    {
      return false;
    }

    // - its replace state is not removed.
    if (_replaceState == ReplaceState::Removed)
    {
      return false;
    }

    auto timing = _effect->GetBasicTiming();

    // An animation effect is in play if all of the following conditions are met:
    // - the animation effect is in the active phase, and
    // - the animation effect is associated with an animation that is not finished.
    if (timing.Phase == AnimationEffectPhase::Active && GetPlayState() != PlayState::Finished)
    {
      return true;
    }

    // An animation effect is current if any of the following conditions are true:
    // - the animation effect is in play, or
    // - the animation effect is associated with an animation with a playback rate > 0 and the animation
    // effect
    //   is in the before phase, or
    if (_playbackRate > 0 && timing.Phase == AnimationEffectPhase::Before)
    {
      return true;
    }

    // - the animation effect is associated with an animation with a playback rate < 0 and the animation
    // effect
    //   is in the after phase.
    if (_playbackRate < 0 && timing.Phase == AnimationEffectPhase::After)
    {
      return true;
    }

    // - the animation effect is associated with an animation not in the idle play state with a non-null
    // associated timeline that is not monotonically increasing.
    if (_timeline && !_timeline->IsMonotonic() && GetPlayState() != PlayState::Idle)
    {
      return true;
    }

    // An animation effect is in effect if its active time, as calculated according to the procedure in
    // § 4.8.3.1 Calculating the active time, is not unresolved.
    if (timing.ActiveTime)
    {
      return true;
    }

    return false;
  }

  bool WebAnimation::IsReplaceable() const noexcept
  {
    // An animation is replaceable if all of the following conditions are true:
    // https://drafts.csswg.org/web-animations/#removing-replaced-animations

    // The existence of the animation is not prescribed by markup. That is, it is not a CSS animation with an
    // owning element, nor a CSS transition with an owning element.
    auto *styleOriginatedAnimation = DynamicDowncast<StyleOriginatedAnimation>(this);
    if (styleOriginatedAnimation && styleOriginatedAnimation->OwningElement())
    {
      return false;
    }

    // The animation's play state is finished.
    if (GetPlayState() != PlayState::Finished)
    {
      return false;
    }

    // The animation's replace state is not removed.
    if (_replaceState == ReplaceState::Removed)
    {
      return false;
    }

    // The animation is associated with a monotonically increasing timeline.
    if (!_timeline)
    {
      return false;
    }

    // The animation has an associated target effect.
    if (!_effect)
    {
      return false;
    }

    // The target effect associated with the animation is in effect.
    if (!_effect->GetBasicTiming().ActiveTime)
    {
      return false;
    }

    // The target effect has an associated target element.
    RefPtr keyframeEffect = ShareRefPtr(GetKeyframeEffect());
    if (!keyframeEffect || !keyframeEffect->Target())
    {
      return false;
    }

    return true;
  }

  void WebAnimation::Persist() noexcept
  {
    SetReplaceState(ReplaceState::Persisted);
  }

  void WebAnimation::SetReplaceState(ReplaceState replaceState) noexcept
  {
    if (_replaceState == replaceState)
    {
      return;
    }

    _replaceState = replaceState;
    UpdateRelevance();
  }

  Seconds WebAnimation::TimeToNextTick() const noexcept
  {
    constexpr auto infinity = Seconds {std::numeric_limits<double>::infinity()};

    if (Pending())
    {
      return Seconds {0};
    }

    // If we're not running, or time is not advancing for this animation, there's no telling when we'll end.
    auto playbackRate = EffectivePlaybackRate();
    if (GetPlayState() != PlayState::Running || !playbackRate)
    {
      return infinity;
    }

    krys_debug_assert(Effect());
    return Effect()->TimeToNextTick(Effect()->GetBasicTiming()) / playbackRate;
  }

  Maybe<Seconds> WebAnimation::ConvertAnimationTimeToTimelineTime(Seconds animationTime) const noexcept
  {
    // https://drafts.csswg.org/web-animations-1/#animation-time-to-timeline-time

    // To convert an animation time to timeline time a time value, time, that is relative to the start time
    // of an animation, animation, perform the following steps:
    //
    // 1. If time is unresolved, return time.
    // 2. If time is infinity, return an unresolved time value.
    // 3. If animation's playback rate is zero, return an unresolved time value.
    // 4. If animation's start time is unresolved, return an unresolved time value.
    if (!_playbackRate || !_startTime || std::isinf(animationTime.count()))
    {
      return null;
    }

    // 5. Return the result of calculating: time × (1 / playback rate) + start time (where playback rate and
    //    start time are the playback rate and start time of animation, respectively).
    return static_cast<Seconds>(*_startTime + animationTime * (1 / _playbackRate));
  }

  bool WebAnimation::IsSkippedContentAnimation() const noexcept
  {
    // FIXME: Animations of skipped content (content-visibility), once there's content visibility.
    return false;
  }

  Maybe<double> WebAnimation::OverallProgress() const noexcept
  {
    // https://drafts.csswg.org/web-animations-2/#the-overall-progress-of-an-animation
    // An animation's overallProgress is the ratio of its current time to its associated effect end.
    //
    // The overallProgress of an animation, animation, is calculated as follows:
    //
    // If any of the following are true:
    //     - animation does not have an associated effect, or
    //     - animation's current time is an unresolved time value,
    // animation's overallProgress is null.
    if (!_effect)
    {
      return null;
    }

    auto currentTime = CurrentTime();
    if (!currentTime)
    {
      return null;
    }

    auto endTime = EffectEndTime();

    // If animation's associated effect end is zero,
    //     - If animation's current time is negative, animation's overallProgress is zero.
    //     - Otherwise, animation's overallProgress is one.
    if (endTime.IsZero())
    {
      return *currentTime < endTime.MatchingZero() ? 0 : 1;
    }

    // If animation's associated effect end is infinite, animation's overallProgress is zero.
    if (endTime.IsInfinity())
    {
      return 0;
    }

    // Otherwise, overallProgress = min(max(current time / animation's associated effect end, 0), 1)
    return std::min(std::max(*currentTime / endTime, 0.0), 1.0);
  }
}

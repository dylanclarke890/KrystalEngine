/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Animation/Timing/WebAnimationTime.hpp"
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
  class ScrollTimeline;
  class WebAnimation;

  /// @brief A document's timelines: it caches the current time for an animation frame, gives pending
  /// animations their start time, and runs the "update animations and send events" procedure.
  // FIXME: Accelerated effect stacks and hidden page suspension, once there are any. (No frame rate
  // alignment: animation frame rates are non-standard.)
  class AnimationTimelinesController final
  {
    KRYS_NON_COPY_MOVABLE(AnimationTimelinesController);

  private:
    Timer _cachedCurrentTimeClearanceTimer;
    List<WeakPtr<AnimationTimeline>> _timelines;
    List<WeakPtr<WebAnimation>> _pendingAnimations;
    List<Ref<ScrollTimeline>> _updatedScrollTimelines;
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
    void RunPostRenderingUpdateTasks() noexcept;

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
    KRYS_NODISCARD bool IsPendingTimelineAttachment(const WebAnimation &animation) const noexcept;
  };
}

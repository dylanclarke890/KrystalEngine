/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Animation/Events/AnimationEventBase.hpp"

namespace krys::boo
{
  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-animationplaybackeventinit
  struct AnimationPlaybackEventInit : public dom::EventInit
  {
    Maybe<WebAnimationTime> CurrentTime;
    Maybe<WebAnimationTime> TimelineTime;
  };

  /// @brief Fired at an animation when its play state changes: `finish`, `cancel` and `remove`.
  /// @see https://drafts.csswg.org/web-animations-1/#animationplaybackevent
  class AnimationPlaybackEvent final : public AnimationEventBase
  {
  private:
    Maybe<WebAnimationTime> _timelineTime;
    Maybe<WebAnimationTime> _currentTime;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = AnimationPlaybackEventInit;

    AnimationPlaybackEvent(dom::DOMStringAtom type, WebAnimation *animation,
                           Maybe<WebAnimationTime> scheduledTime, Maybe<WebAnimationTime> timelineTime,
                           Maybe<WebAnimationTime> currentTime) noexcept;

    /// @see https://drafts.csswg.org/web-animations-1/#dom-animationplaybackevent-animationplaybackevent
    explicit AnimationPlaybackEvent(dom::DOMStringAtom type,
                                    const AnimationPlaybackEventInit &eventInitDict = {}) noexcept;

    ~AnimationPlaybackEvent() noexcept override;

    KRYS_NODISCARD bool IsAnimationPlaybackEvent() const noexcept final
    {
      return true;
    }

    /// @see https://drafts.csswg.org/web-animations-1/#dom-animationplaybackevent-timelinetime
    KRYS_NODISCARD Maybe<WebAnimationTime> TimelineTime() const noexcept
    {
      return _timelineTime;
    }

    /// @see https://drafts.csswg.org/web-animations-1/#dom-animationplaybackevent-currenttime
    KRYS_NODISCARD Maybe<WebAnimationTime> CurrentTime() const noexcept
    {
      return _currentTime;
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_ANIMATION_EVENT_BASE(krys::boo::AnimationPlaybackEvent,
                                                 IsAnimationPlaybackEvent())

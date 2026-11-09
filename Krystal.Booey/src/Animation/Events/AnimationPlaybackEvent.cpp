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

#include "Krystal.Booey/Animation/Events/AnimationPlaybackEvent.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD Maybe<Seconds> ToSeconds(const Maybe<WebAnimationTime> &time) noexcept
    {
      if (time)
      {
        return time->Time();
      }
      return null;
    }
  }

  AnimationPlaybackEvent::AnimationPlaybackEvent(dom::DOMStringAtom type,
                                                 const AnimationPlaybackEventInit &eventInitDict) noexcept
      : AnimationEventBase(type, eventInitDict), _timelineTime(eventInitDict.TimelineTime),
        _currentTime(eventInitDict.CurrentTime)
  {
  }

  AnimationPlaybackEvent::AnimationPlaybackEvent(dom::DOMStringAtom type, WebAnimation *animation,
                                                 Maybe<WebAnimationTime> scheduledTime,
                                                 Maybe<WebAnimationTime> timelineTime,
                                                 Maybe<WebAnimationTime> currentTime) noexcept
      : AnimationEventBase(type, animation, ToSeconds(scheduledTime)), _timelineTime(timelineTime),
        _currentTime(currentTime)
  {
  }

  AnimationPlaybackEvent::~AnimationPlaybackEvent() noexcept = default;
}

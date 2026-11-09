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

#include "Krystal.Booey/Animation/Events/AnimationEventBase.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"

namespace krys::boo
{
  AnimationEventBase::AnimationEventBase(dom::DOMStringAtom type, WebAnimation *animation,
                                         Maybe<Seconds> scheduledTime) noexcept
      : Event(type, dom::EventInit {.Bubbles = true, .Cancelable = false}),
        _animation(ShareRefPtr(animation)), _scheduledTime(scheduledTime)
  {
    _isTrusted = true;
    _initialized = true;
  }

  AnimationEventBase::AnimationEventBase(dom::DOMStringAtom type,
                                         const dom::EventInit &eventInitDict) noexcept
      : Event(type, eventInitDict)
  {
  }

  AnimationEventBase::~AnimationEventBase() noexcept = default;
}

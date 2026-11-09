/*
 * Copyright 2008, The Android Open Source Project
 * Copyright (C) 2012 Research In Motion Limited. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
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

#include "Krystal.Booey/DOM/Events/TouchEvent.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD MouseEventInit ToMouseEventInit(const TouchEventInit &initializer) noexcept
    {
      MouseEventInit init;
      static_cast<EventModifierInit &>(init) = initializer;
      return init;
    }

    KRYS_NODISCARD UIEventInit TrustedInit(DOMStringAtom type) noexcept
    {
      // https://w3c.github.io/touch-events/#list-of-touchevent-types
      UIEventInit init;
      init.Bubbles = true;
      init.Cancelable = type != EventNames::TouchCancel;
      init.Composed = true;
      return init;
    }
  }

  TouchEvent::TouchEvent(DOMStringAtom type, const TouchEventInit &initializer) noexcept
      : MouseRelatedEvent(type, ToMouseEventInit(initializer)),
        _touches(TouchList::Create(initializer.Touches)),
        _targetTouches(TouchList::Create(initializer.TargetTouches)),
        _changedTouches(TouchList::Create(initializer.ChangedTouches))
  {
  }

  TouchEvent::TouchEvent(TouchList *touches, TouchList *targetTouches, TouchList *changedTouches,
                         DOMStringAtom type, LocalFrameView *view, const DoublePoint &globalLocation,
                         EnumSet<Modifier> modifiers) noexcept
      : MouseRelatedEvent(type, TrustedInit(type), view, globalLocation, {}, 0, 0, modifiers, Simulated::No),
        _touches(ShareRefPtr(touches)), _targetTouches(ShareRefPtr(targetTouches)),
        _changedTouches(ShareRefPtr(changedTouches))
  {
    _isTrusted = true;
  }

  TouchEvent::~TouchEvent() noexcept = default;

  Ref<TouchEvent> TouchEvent::Create(TouchList *touches, TouchList *targetTouches, TouchList *changedTouches,
                                     DOMStringAtom type, LocalFrameView *view,
                                     const DoublePoint &globalLocation, EnumSet<Modifier> modifiers) noexcept
  {
    return AdoptRef(
      *new TouchEvent(touches, targetTouches, changedTouches, type, view, globalLocation, modifiers));
  }
}

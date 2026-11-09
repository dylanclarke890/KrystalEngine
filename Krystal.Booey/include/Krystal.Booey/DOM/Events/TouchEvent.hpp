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

#pragma once

#include "Krystal.Booey/DOM/Events/MouseRelatedEvent.hpp"
#include "Krystal.Booey/DOM/Events/TouchList.hpp"
#include "Krystal.Booey/DOM/Events/UIEvent.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Touch;

  /// @see https://w3c.github.io/touch-events/#touchevent-interface
  struct TouchEventInit : EventModifierInit
  {
    List<Ref<Touch>> Touches;
    List<Ref<Touch>> TargetTouches;
    List<Ref<Touch>> ChangedTouches;
  };

  /// @brief A change to the touches on the screen: `touchstart`, `touchmove`, `touchend` and `touchcancel`.
  /// @see https://w3c.github.io/touch-events/#touchevent-interface
  class TouchEvent final : public MouseRelatedEvent
  {
  private:
    RefPtr<TouchList> _touches;
    RefPtr<TouchList> _targetTouches;
    RefPtr<TouchList> _changedTouches;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = TouchEventInit;

    /// @see https://w3c.github.io/touch-events/#dom-touchevent-touchevent
    TouchEvent(DOMStringAtom type, const TouchEventInit &initializer = {}) noexcept;

    ~TouchEvent() noexcept override;

    /// @brief A trusted event, which happened in @p view.
    KRYS_NODISCARD static Ref<TouchEvent> Create(TouchList *touches, TouchList *targetTouches,
                                                 TouchList *changedTouches, DOMStringAtom type,
                                                 LocalFrameView *view, const DoublePoint &globalLocation,
                                                 EnumSet<Modifier> modifiers) noexcept;

    /// @see https://w3c.github.io/touch-events/#dom-touchevent-touches
    KRYS_NODISCARD TouchList *Touches() const noexcept
    {
      return _touches.get();
    }

    /// @see https://w3c.github.io/touch-events/#dom-touchevent-targettouches
    KRYS_NODISCARD TouchList *TargetTouches() const noexcept
    {
      return _targetTouches.get();
    }

    /// @see https://w3c.github.io/touch-events/#dom-touchevent-changedtouches
    KRYS_NODISCARD TouchList *ChangedTouches() const noexcept
    {
      return _changedTouches.get();
    }

    void SetTouches(RefPtr<TouchList> &&touches) noexcept
    {
      _touches = krys::move(touches);
    }

    void SetTargetTouches(RefPtr<TouchList> &&targetTouches) noexcept
    {
      _targetTouches = krys::move(targetTouches);
    }

    void SetChangedTouches(RefPtr<TouchList> &&changedTouches) noexcept
    {
      _changedTouches = krys::move(changedTouches);
    }

    KRYS_NODISCARD bool IsTouchEvent() const noexcept override
    {
      return true;
    }

  private:
    TouchEvent(TouchList *touches, TouchList *targetTouches, TouchList *changedTouches, DOMStringAtom type,
               LocalFrameView *view, const DoublePoint &globalLocation, EnumSet<Modifier> modifiers) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::TouchEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsTouchEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();

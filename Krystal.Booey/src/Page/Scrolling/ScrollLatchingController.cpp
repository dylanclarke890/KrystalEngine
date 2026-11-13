/*
 * Copyright (C) 2020-2022 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Page/Scrolling/ScrollLatchingController.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/Events/PlatformWheelEvent.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollableArea.hpp"
#include <cmath>

namespace krys::boo
{
  void ScrollLatchingController::Clear() noexcept
  {
    _cumulativeEventDelta = {};
    _state.reset();
  }

  void ScrollLatchingController::ReceivedWheelEvent(const PlatformWheelEvent &wheelEvent) noexcept
  {
    // The state lasts until the next gesture or wheel notch, with no timer: a gesture's momentum follows its
    // last step, with no end between them to wait past.
    if (!wheelEvent.UseLatchedEventElement() || wheelEvent.IsGestureStart())
    {
      Clear();
    }

    if (wheelEvent.IsGestureStart() || !wheelEvent.UseLatchedEventElement())
    {
      _cumulativeEventDelta = wheelEvent.Delta();
    }
    else
    {
      _cumulativeEventDelta += wheelEvent.Delta();
    }
  }

  dom::Element *ScrollLatchingController::LatchedElement() const noexcept
  {
    return _state ? _state->WheelEventElement.get() : nullptr;
  }

  bool
    ScrollLatchingController::LatchingAllowsScrolling(WeakPtr<ScrollableArea> &latchedScroller) const noexcept
  {
    if (!_state)
    {
      return true;
    }

    latchedScroller = _state->GestureScrollableArea;
    return static_cast<bool>(_state->GestureScrollableArea);
  }

  void ScrollLatchingController::UpdateAndFetchLatchingState(const LocalFrameView &view,
                                                             const PlatformWheelEvent &wheelEvent,
                                                             RefPtr<dom::Element> &latchedElement,
                                                             WeakPtr<ScrollableArea> &scrollableArea) noexcept
  {
    if (!wheelEvent.UseLatchedEventElement())
    {
      return;
    }

    if (wheelEvent.IsGestureStart() && !_state)
    {
      LatchingState state;
      state.WheelEventElement = latchedElement ? CreateWeakPtr(latchedElement.get()) : nullptr;
      if (ShouldLatchToScrollableArea(view, scrollableArea.get(), _cumulativeEventDelta))
      {
        state.GestureScrollableArea = scrollableArea;
      }
      _state = krys::move(state);
      return;
    }

    if (!_state)
    {
      return;
    }

    // We may not have latched at gesture start because of small deltas. Re-evaluate latching based on
    // accumulated delta.
    if (wheelEvent.IsGestureContinuation() && !_state->GestureScrollableArea
        && ShouldLatchToScrollableArea(view, scrollableArea.get(), _cumulativeEventDelta))
    {
      _state->GestureScrollableArea = scrollableArea;
    }

    latchedElement = ShareRefPtr(_state->WheelEventElement.get());
    scrollableArea = _state->GestureScrollableArea;
  }

  void ScrollLatchingController::RemoveLatchingStateForTarget(const dom::Element &element) noexcept
  {
    if (_state && _state->WheelEventElement.get() == &element)
    {
      _state.reset();
    }
  }

  bool ScrollLatchingController::ShouldLatchToScrollableArea(const LocalFrameView &view,
                                                             ScrollableArea *scrollableArea,
                                                             FloatSize scrollDelta) noexcept
  {
    if (!scrollableArea)
    {
      return false;
    }

    // A gesture's start moves nothing, so it latches once its first step shows which way it goes.
    if (scrollDelta.IsZero())
    {
      return false;
    }

    // We always allow the main frame to receive wheel events to permit rubber-banding.
    if (scrollableArea == &static_cast<const ScrollableArea &>(view))
    {
      return true;
    }

    // A box latches when it can scroll the gesture's way, whether or not it shows a scrollbar (WebKit's
    // needs one).
    bool predominantlyVertical = std::abs(scrollDelta.Height()) > std::abs(scrollDelta.Width());
    if (!predominantlyVertical && scrollDelta.Width())
    {
      return scrollDelta.Width() < 0 ? !scrollableArea->ScrolledToRight() : !scrollableArea->ScrolledToLeft();
    }

    return scrollDelta.Height() < 0 ? !scrollableArea->ScrolledToBottom() : !scrollableArea->ScrolledToTop();
  }
}

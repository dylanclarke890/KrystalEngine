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

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::web
{
  class Element;
  class FrameView;
  class PlatformWheelEvent;
  class ScrollableArea;

  /// @brief Keeps a touchpad's gesture, and the momentum after it, on the element and scroller it started
  /// over: the rest of its events go to that element and scroll only that scroller, wherever the pointer goes
  /// and however far the scroller has gone.
  /// @note There's no frame tree, so there's one latching state rather than WebKit's stack of them.
  class ScrollLatchingController
  {
    KRYS_NON_COPY_MOVABLE(ScrollLatchingController);

  private:
    struct LatchingState
    {
      WeakPtr<Element> WheelEventElement;
      WeakPtr<ScrollableArea> GestureScrollableArea;
    };

    SizeF _cumulativeEventDelta;
    Maybe<LatchingState> _state;

  public:
    ScrollLatchingController() noexcept = default;

    /// @brief Forgets the latched element and scroller.
    void Clear() noexcept;

    /// @brief Starts a new gesture's state, or forgets it for an event that isn't part of one; and adds up
    /// the gesture's deltas.
    void ReceivedWheelEvent(const PlatformWheelEvent &wheelEvent) noexcept;

    /// @brief The deltas of the gesture so far.
    KRYS_NODISCARD SizeF CumulativeEventDelta() const noexcept
    {
      return _cumulativeEventDelta;
    }

    /// @brief The element the gesture started over, if it's still latched.
    KRYS_NODISCARD Element *LatchedElement() const noexcept;

    /// @brief Whether the event may scroll: when nothing is latched, or a scroller is (@p latchedScroller).
    KRYS_NODISCARD bool LatchingAllowsScrolling(WeakPtr<ScrollableArea> &latchedScroller) const noexcept;

    /// @brief Latches the event's element and scroller at a gesture's start (or once its deltas are enough
    /// to scroll), and for the gesture's later events replaces them with the latched ones.
    void UpdateAndFetchLatchingState(const FrameView &view, const PlatformWheelEvent &wheelEvent,
                                     RefPtr<Element> &latchedElement,
                                     WeakPtr<ScrollableArea> &scrollableArea) noexcept;

    /// @brief Forgets the latched state if @p element, which is leaving the document, was its target.
    void RemoveLatchingStateForTarget(const Element &element) noexcept;

  private:
    KRYS_NODISCARD static bool ShouldLatchToScrollableArea(const FrameView &view,
                                                           ScrollableArea *scrollableArea,
                                                           SizeF scrollDelta) noexcept;
  };
}

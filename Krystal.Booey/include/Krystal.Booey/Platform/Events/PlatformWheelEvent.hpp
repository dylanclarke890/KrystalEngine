/*
 * Copyright (C) 2004-2016 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntPoint.hpp"
#include "Krystal.Booey/Platform/Events/PlatformEvent.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief What happened to an event the host sent: whether it reached the DOM, and whether a listener
  /// cancelled it or a default action handled it.
  enum class EventHandling : uint8
  {
    DispatchedToDOM,
    DefaultPrevented,
    DefaultHandled,
  };

  // The ScrollByPixelWheelEvent is a fine-grained event that specifies the precise number of pixels to
  // scroll. It is sent directly by touch pads on macOS, or synthesized when platforms generate line-by-line
  // scrolling events.
  //
  // The ScrollByPageWheelEvent indicates that the wheel event should scroll an entire page.
  // In this case, built in paging behavior is used to page up and down.
  // This yields the same behavior as clicking in a scrollbar track to page up and down.
  enum class PlatformWheelEventGranularity : uint8
  {
    ScrollByPageWheelEvent,
    ScrollByPixelWheelEvent,
  };

  /// @brief Where a wheel event is in a gesture (a touchpad's pan) or in the momentum after it: its start, a
  /// step, or its end; or in none, as a wheel's notches are.
  /// @note A gesture that goes on into momentum has no `Ended`: its momentum's `Began` follows its last
  /// `Changed`. So a gesture is over at an `Ended` or `Cancelled` of either.
  enum class PlatformWheelEventPhase : uint8
  {
    None,
    Began,
    Changed,
    Ended,
    Cancelled,
  };

  // How many pixels should we scroll per line? Gecko uses the height of the
  // current line, which means scroll distance changes as you go through the
  // page or go to different pages. IE 7 is ~50 px/line, although the value
  // seems to vary slightly by page and zoom level. Since IE 7 has a
  // smoothing algorithm on scrolling, it can get away with slightly larger
  // scroll values without feeling jerky. Here we use 100 px per three lines
  // (the default scroll amount on Windows is three lines per wheel tick).
  constexpr float ScrollbarPixelsPerLine = 100.0f / 3.0f;

  /// @brief A turn of the mouse wheel, or a step of a touchpad's pan, from the host.
  /// @note The deltas are positive when scrolling up or left: the direction the content moves, the opposite
  /// of the DOM's `deltaX` and `deltaY`.
  // FIXME: Scrolling velocities and inverted directions.
  class PlatformWheelEvent : public PlatformEvent
  {
  protected:
    PlatformWheelEventGranularity _granularity {PlatformWheelEventGranularity::ScrollByPixelWheelEvent};
    IntPoint _position;
    IntPoint _globalPosition;
    float _deltaX {0};
    float _deltaY {0};
    float _wheelTicksX {0};
    float _wheelTicksY {0};
    PlatformWheelEventPhase _phase {PlatformWheelEventPhase::None};
    PlatformWheelEventPhase _momentumPhase {PlatformWheelEventPhase::None};
    bool _hasPreciseScrollingDeltas {false};
    bool _isTouchScroll {false};

  public:
    PlatformWheelEvent() noexcept : PlatformEvent(Type::Wheel)
    {
    }

    /// @param position The point in the view's coordinates.
    /// @param globalPosition The point in the screen's coordinates.
    /// @param deltaX How far to scroll: pixels, or a page's direction when @p granularity is by page.
    /// @param wheelTicksX How far the wheel turned, in notches.
    PlatformWheelEvent(IntPoint position, IntPoint globalPosition, float deltaX, float deltaY,
                       float wheelTicksX, float wheelTicksY, PlatformWheelEventGranularity granularity,
                       EnumSet<PlatformEvent::Modifier> modifiers, MonotonicTime::duration timestamp) noexcept
        : PlatformEvent(Type::Wheel, modifiers, timestamp), _granularity(granularity), _position(position),
          _globalPosition(globalPosition), _deltaX(deltaX), _deltaY(deltaY), _wheelTicksX(wheelTicksX),
          _wheelTicksY(wheelTicksY)
    {
    }

    KRYS_NODISCARD PlatformWheelEvent CopySwappingDirection() const noexcept
    {
      PlatformWheelEvent copy = *this;
      std::swap(copy._deltaX, copy._deltaY);
      std::swap(copy._wheelTicksX, copy._wheelTicksY);
      return copy;
    }

    /// @brief This event, as a step of a touch's pan or fling: a gesture that snaps only once it ends.
    KRYS_NODISCARD PlatformWheelEvent CopyAsTouchScroll() const noexcept
    {
      PlatformWheelEvent copy = *this;
      copy._isTouchScroll = true;
      return copy;
    }
    KRYS_NODISCARD bool IsTouchScroll() const noexcept
    {
      return _isTouchScroll;
    }

    /// @brief This event, as a step of a touchpad's pan: in @p phase of the gesture or @p momentumPhase of
    /// the momentum after it (the other being `None`), with deltas in pixels the content follows rather than
    /// notches.
    KRYS_NODISCARD PlatformWheelEvent CopyAsTouchpadPan(PlatformWheelEventPhase phase,
                                                        PlatformWheelEventPhase momentumPhase) const noexcept
    {
      PlatformWheelEvent copy = *this;
      copy._phase = phase;
      copy._momentumPhase = momentumPhase;
      copy._hasPreciseScrollingDeltas = true;
      return copy;
    }

    KRYS_NODISCARD PlatformWheelEventPhase Phase() const noexcept
    {
      return _phase;
    }
    KRYS_NODISCARD PlatformWheelEventPhase MomentumPhase() const noexcept
    {
      return _momentumPhase;
    }
    /// @brief Whether the deltas are the pixels the content follows (a touchpad's), rather than notches of a
    /// wheel made into pixels.
    KRYS_NODISCARD bool HasPreciseScrollingDeltas() const noexcept
    {
      return _hasPreciseScrollingDeltas;
    }

    /// @brief Whether the event is a step of a gesture (a touch's or a touchpad's pan, or the momentum
    /// after it), which moves freely and snaps once it ends, rather than a notch, which goes to the next
    /// snap position.
    KRYS_NODISCARD bool IsGestureScroll() const noexcept
    {
      return _isTouchScroll || _phase != PlatformWheelEventPhase::None
             || _momentumPhase != PlatformWheelEventPhase::None;
    }
    /// @brief Whether the event ends a touchpad's gesture, and the momentum after it.
    KRYS_NODISCARD bool EndsGesture() const noexcept
    {
      auto ends = [](PlatformWheelEventPhase phase)
      {
        return phase == PlatformWheelEventPhase::Ended || phase == PlatformWheelEventPhase::Cancelled;
      };
      return ends(_phase) || ends(_momentumPhase);
    }

    KRYS_NODISCARD PlatformWheelEvent CopyWithDelta(FloatSize delta) const noexcept
    {
      PlatformWheelEvent copy = *this;
      copy._deltaX = delta.Width();
      copy._deltaY = delta.Height();
      return copy;
    }

    // PlatformWindow coordinates.
    KRYS_NODISCARD const IntPoint &Position() const noexcept
    {
      return _position;
    }
    // Screen coordinates.
    KRYS_NODISCARD const IntPoint &GlobalPosition() const noexcept
    {
      return _globalPosition;
    }

    KRYS_NODISCARD float DeltaX() const noexcept
    {
      return _deltaX;
    }
    KRYS_NODISCARD float DeltaY() const noexcept
    {
      return _deltaY;
    }
    KRYS_NODISCARD FloatSize Delta() const noexcept
    {
      return {_deltaX, _deltaY};
    }

    KRYS_NODISCARD float WheelTicksX() const noexcept
    {
      return _wheelTicksX;
    }
    KRYS_NODISCARD float WheelTicksY() const noexcept
    {
      return _wheelTicksY;
    }

    KRYS_NODISCARD PlatformWheelEventGranularity Granularity() const noexcept
    {
      return _granularity;
    }
  };
}

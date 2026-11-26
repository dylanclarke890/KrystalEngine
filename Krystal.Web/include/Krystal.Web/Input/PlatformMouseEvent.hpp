/*
 * Copyright (C) 2004-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Web/Events/Pointer/PointerEventTypeNames.hpp"
#include "Krystal.Web/Input/MouseEventTypes.hpp"
#include "Krystal.Web/Input/PlatformEvent.hpp"
#include "Krystal.Web/Input/PointerId.hpp"

namespace krys::web
{
  /// @brief A mouse move, press or release from the host (or a pen's, or a tap's standing in for a click).
  class PlatformMouseEvent : public PlatformEvent
  {
  protected:
    MouseButton _button {MouseButton::None};
    SyntheticClickType _syntheticClickType {SyntheticClickType::NoTap};
    PointD _position;
    PointD _globalPosition;
    PointD _movementDelta;
    PointD _unadjustedMovementDelta;
    double _force {0};
    PenOrientation _penOrientation;
    web::PointerId _pointerId {MousePointerId};
    DOMString _pointerType {pointer_events::MousePointerEventType()};
    int32 _clickCount {0};
    uint16 _buttons {0};
    List<PlatformMouseEvent> _coalescedEvents;
    List<PlatformMouseEvent> _predictedEvents;

  public:
    PlatformMouseEvent() noexcept : PlatformEvent(Type::MouseMoved)
    {
    }

    /// @param position The point in the view's coordinates.
    /// @param globalPosition The point in the screen's coordinates.
    /// @param buttons The buttons held, as the DOM's `buttons` bit set.
    /// @param force The pressure, from 0 to 1, for a pressure sensitive device.
    PlatformMouseEvent(const PointD &position, const PointD &globalPosition, MouseButton button,
                       PlatformEvent::Type type, int32 clickCount, EnumSet<PlatformEvent::Modifier> modifiers,
                       MonotonicTime::duration timestamp, uint16 buttons = 0,
                       const PointD &movementDelta = {}, double force = 0,
                       SyntheticClickType syntheticClickType = SyntheticClickType::NoTap,
                       web::PointerId pointerId = MousePointerId,
                       DOMString pointerType = pointer_events::MousePointerEventType()) noexcept
        : PlatformEvent(type, modifiers, timestamp), _button(button), _syntheticClickType(syntheticClickType),
          _position(position), _globalPosition(globalPosition), _movementDelta(movementDelta), _force(force),
          _pointerId(pointerId), _pointerType(krys::move(pointerType)), _clickCount(clickCount),
          _buttons(buttons)
    {
    }

    // This position is relative to the view. Use the view's windowToContents() to convert it to into the
    // contents of a given view.
    KRYS_NODISCARD const PointD &Position() const noexcept
    {
      return _position;
    }

    KRYS_NODISCARD const PointD &GlobalPosition() const noexcept
    {
      return _globalPosition;
    }

    KRYS_NODISCARD const PointD &MovementDelta() const noexcept
    {
      return _movementDelta;
    }

    // Unaccelerated pointer movement
    KRYS_NODISCARD const PointD &UnadjustedMovementDelta() const noexcept
    {
      return _unadjustedMovementDelta;
    }

    void SetUnadjustedMovementDelta(const PointD &unadjustedMovementDelta) noexcept
    {
      _unadjustedMovementDelta = unadjustedMovementDelta;
    }

    KRYS_NODISCARD MouseButton Button() const noexcept
    {
      return _button;
    }

    KRYS_NODISCARD uint16 Buttons() const noexcept
    {
      return _buttons;
    }

    KRYS_NODISCARD int32 ClickCount() const noexcept
    {
      return _clickCount;
    }

    KRYS_NODISCARD double Force() const noexcept
    {
      return _force;
    }

    /// @brief How the pen is held, for a pen's events.
    KRYS_NODISCARD const PenOrientation &GetPenOrientation() const noexcept
    {
      return _penOrientation;
    }

    void SetPenOrientation(const PenOrientation &penOrientation) noexcept
    {
      _penOrientation = penOrientation;
    }

    KRYS_NODISCARD SyntheticClickType GetSyntheticClickType() const noexcept
    {
      return _syntheticClickType;
    }

    KRYS_NODISCARD web::PointerId PointerId() const noexcept
    {
      return _pointerId;
    }

    KRYS_NODISCARD const DOMString &PointerType() const noexcept
    {
      return _pointerType;
    }

    /// @brief The moves this one stands for, oldest first, when the host merged several into it (itself, if
    /// it merged none).
    KRYS_NODISCARD const List<PlatformMouseEvent> &CoalescedEvents() const noexcept
    {
      return _coalescedEvents;
    }

    void SetCoalescedEvents(List<PlatformMouseEvent> coalescedEvents) noexcept
    {
      _coalescedEvents = krys::move(coalescedEvents);
    }

    /// @brief Where the host predicts the pointer will move next, soonest first.
    KRYS_NODISCARD const List<PlatformMouseEvent> &PredictedEvents() const noexcept
    {
      return _predictedEvents;
    }

    void SetPredictedEvents(List<PlatformMouseEvent> predictedEvents) noexcept
    {
      _predictedEvents = krys::move(predictedEvents);
    }
  };
}

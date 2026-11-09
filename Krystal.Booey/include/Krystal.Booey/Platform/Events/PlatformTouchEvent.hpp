/*
    Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#pragma once

#include "Krystal.Booey/Platform/Events/PlatformEvent.hpp"
#include "Krystal.Booey/Platform/Events/PlatformTouchPoint.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief A change to the touches on the host's touch screen: every touch on it, each with its state.
  class PlatformTouchEvent : public PlatformEvent
  {
  protected:
    List<PlatformTouchPoint> _touchPoints;
    List<PlatformTouchEvent> _coalescedEvents;
    List<PlatformTouchEvent> _predictedEvents;

  public:
    PlatformTouchEvent() noexcept : PlatformEvent(PlatformEvent::Type::TouchStart)
    {
    }

    /// @param type `TouchStart`, `TouchMove`, `TouchEnd` or `TouchCancel`, as the touches changed.
    /// @param touchPoints Every touch on the screen, those that didn't change as `TouchStationary`.
    PlatformTouchEvent(PlatformEvent::Type type, List<PlatformTouchPoint> touchPoints,
                       EnumSet<PlatformEvent::Modifier> modifiers, MonotonicTime::duration timestamp) noexcept
        : PlatformEvent(type, modifiers, timestamp), _touchPoints(krys::move(touchPoints))
    {
    }

    KRYS_NODISCARD const List<PlatformTouchPoint> &TouchPoints() const noexcept
    {
      return _touchPoints;
    }

    /// @brief The changes this one stands for, oldest first, when the host merged several into it.
    KRYS_NODISCARD const List<PlatformTouchEvent> &CoalescedEvents() const noexcept
    {
      return _coalescedEvents;
    }

    void SetCoalescedEvents(List<PlatformTouchEvent> coalescedEvents) noexcept
    {
      _coalescedEvents = krys::move(coalescedEvents);
    }

    /// @brief Where the host predicts the touches will move next, soonest first.
    KRYS_NODISCARD const List<PlatformTouchEvent> &PredictedEvents() const noexcept
    {
      return _predictedEvents;
    }

    void SetPredictedEvents(List<PlatformTouchEvent> predictedEvents) noexcept
    {
      _predictedEvents = krys::move(predictedEvents);
    }
  };
}

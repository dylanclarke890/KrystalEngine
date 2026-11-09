/*
 * Copyright (C) 2001 Peter Kelly (pmk@post.com)
 * Copyright (C) 2001 Tobias Anton (anton@stud.fbi.fh-darmstadt.de)
 * Copyright (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
 * Copyright (C) 2003, 2005, 2006, 2008, 2010, 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Samsung Electronics. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/DOM/Events/WheelEvent.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD uint32 DetermineDeltaMode(const PlatformWheelEvent &event) noexcept
    {
      return event.Granularity() == PlatformWheelEventGranularity::ScrollByPageWheelEvent
               ? WheelEvent::DOM_DELTA_PAGE
               : WheelEvent::DOM_DELTA_PIXEL;
    }

    KRYS_NODISCARD UIEventInit TrustedInit(WheelEvent::IsCancelable isCancelable) noexcept
    {
      UIEventInit init;
      init.Bubbles = true;
      init.Cancelable = isCancelable == WheelEvent::IsCancelable::Yes;
      init.Composed = true;
      return init;
    }

    KRYS_NODISCARD DoublePoint ToDoublePoint(const IntPoint &point) noexcept
    {
      return {static_cast<double>(point.X()), static_cast<double>(point.Y())};
    }
  }

  WheelEvent::WheelEvent(DOMStringAtom type, const WheelEventInit &initializer) noexcept
      : MouseEvent(type, initializer), _deltaX(initializer.DeltaX), _deltaY(initializer.DeltaY),
        _deltaZ(initializer.DeltaZ), _deltaMode(initializer.DeltaMode)
  {
  }

  WheelEvent::WheelEvent(const PlatformWheelEvent &event, LocalFrameView *view,
                         IsCancelable isCancelable) noexcept
      : MouseEvent(DOMStringAtom(EventNames::Wheel), TrustedInit(isCancelable), view,
                   ToDoublePoint(event.GlobalPosition()), ToDoublePoint(event.Position()), 0, 0,
                   event.Modifiers(), MouseButton::Left, 0, nullptr, 0, SyntheticClickType::NoTap, {}, {},
                   Simulated::No),
        _deltaX(-event.DeltaX()), _deltaY(-event.DeltaY()), _deltaMode(DetermineDeltaMode(event)),
        _underlyingPlatformEvent(event)
  {
    _createTime = event.Timestamp();
  }

  Ref<WheelEvent> WheelEvent::Create(const PlatformWheelEvent &event, LocalFrameView *view,
                                     IsCancelable isCancelable) noexcept
  {
    return AdoptRef(*new WheelEvent(event, view, isCancelable));
  }
}

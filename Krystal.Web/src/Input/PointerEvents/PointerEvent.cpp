/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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

#include "Krystal.Web/Input/PointerEvents/PointerEvent.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Debug.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Web/Platform/Events/PlatformTouchEvent.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace krys::web
{
  namespace
  {
    constexpr double PiOverTwo = std::numbers::pi / 2;
    constexpr double RadiansPerDegree = std::numbers::pi / 180;
    constexpr double DegreesPerRadian = 180 / std::numbers::pi;
    constexpr double RadiansPerTurn = 2 * std::numbers::pi;

    // The touches' pointer ids follow the mouse's and the pen's: the host numbers its touches from 0.
    constexpr web::PointerId TouchMinimumPointerId = PenPointerId + 1;

    // https://w3c.github.io/pointerevents/#dom-pointerevent-width
    // If the hardware cannot report contact geometry, a default value of 1 is used.
    KRYS_NODISCARD double TouchContactLength(double radius) noexcept
    {
      return radius > 0 ? 2 * radius : 1;
    }

    KRYS_NODISCARD DOMStringAtom PointerEventType(PlatformTouchPoint::State state) noexcept
    {
      switch (state)
      {
        case PlatformTouchPoint::TouchPressed:    return event_names::PointerDown;
        case PlatformTouchPoint::TouchMoved:
        case PlatformTouchPoint::TouchStationary: return event_names::PointerMove;
        case PlatformTouchPoint::TouchReleased:   return event_names::PointerUp;
        case PlatformTouchPoint::TouchCancelled:  return event_names::PointerCancel;
        case PlatformTouchPoint::TouchStateEnd:   break;
      }
      krys_debug_assert_msg(false, "Not reached.");
      return null;
    }

    KRYS_NODISCARD UIEventInit TrustedInit(bool canBubble, bool isCancelable, bool isComposed,
                                           int32 detail = 0) noexcept
    {
      UIEventInit init;
      init.Bubbles = canBubble;
      init.Cancelable = isCancelable;
      init.Composed = isComposed;
      init.Detail = detail;
      return init;
    }

    KRYS_NODISCARD List<Ref<PointerEvent>> CreateCoalescedPointerEvents(const DOMStringAtom &type,
                                                                        MouseButton button,
                                                                        const MouseEvent &mouseEvent,
                                                                        web::PointerId pointerId,
                                                                        const DOMString &pointerType) noexcept
    {
      if (type != event_names::PointerMove && type != event_names::PointerRawUpdate)
      {
        return {};
      }

      // Populated in accordance with the
      // https://www.w3.org/TR/pointerevents3/#populating-and-maintaining-the-coalesced-and-predicted-events-lists
      // spec.

      List<Ref<PointerEvent>> result;
      for (auto &coalescedMouseEvent : mouseEvent.CoalescedEvents())
      {
        result.push_back(
          PointerEvent::Create(type, button, *coalescedMouseEvent, pointerId, pointerType, false, false));
      }
      return result;
    }

    KRYS_NODISCARD List<Ref<PointerEvent>> CreatePredictedPointerEvents(const DOMStringAtom &type,
                                                                        MouseButton button,
                                                                        const MouseEvent &mouseEvent,
                                                                        web::PointerId pointerId,
                                                                        const DOMString &pointerType) noexcept
    {
      if (type != event_names::PointerMove)
      {
        return {};
      }

      // Populated in accordance with the
      // https://www.w3.org/TR/pointerevents3/#populating-and-maintaining-the-coalesced-and-predicted-events-lists
      // spec.

      List<Ref<PointerEvent>> result;
      for (auto &predictedMouseEvent : mouseEvent.PredictedEvents())
      {
        result.push_back(
          PointerEvent::Create(type, button, *predictedMouseEvent, pointerId, pointerType, false, false));
      }
      return result;
    }
  }

  DOMStringAtom PointerEvent::TypeFromMouseEventType(const DOMStringAtom &mouseEventType) noexcept
  {
    if (mouseEventType == event_names::AuxClick)
    {
      return event_names::AuxClick;
    }
    if (mouseEventType == event_names::Click)
    {
      return event_names::Click;
    }
    if (mouseEventType == event_names::ContextMenu)
    {
      return event_names::ContextMenu;
    }
    if (mouseEventType == event_names::MouseDown)
    {
      return event_names::PointerDown;
    }
    if (mouseEventType == event_names::MouseOver)
    {
      return event_names::PointerOver;
    }
    if (mouseEventType == event_names::MouseEnter)
    {
      return event_names::PointerEnter;
    }
    if (mouseEventType == event_names::MouseMove)
    {
      return event_names::PointerMove;
    }
    if (mouseEventType == event_names::MouseLeave)
    {
      return event_names::PointerLeave;
    }
    if (mouseEventType == event_names::MouseOut)
    {
      return event_names::PointerOut;
    }
    if (mouseEventType == event_names::MouseUp)
    {
      return event_names::PointerUp;
    }

    return null;
  }

  bool PointerEvent::TypeRequiresResolvedButton(const DOMStringAtom &type) noexcept
  {
    return type == event_names::PointerUp || type == event_names::PointerDown || type == event_names::Click
           || type == event_names::AuxClick || type == event_names::ContextMenu;
  }

  Ref<PointerEvent> PointerEvent::CreateForPointerCapture(DOMStringAtom type, Window *view,
                                                          web::PointerId pointerId, bool isPrimary,
                                                          const DOMString &pointerType) noexcept
  {
    PointerEventInit initializer;
    initializer.View = view;
    initializer.Bubbles = true;
    initializer.PointerId = static_cast<int32>(pointerId);
    initializer.IsPrimary = isPrimary;
    initializer.PointerType = pointerType;
    initializer.Composed = true;
    Ref<PointerEvent> event = AdoptRef(*new PointerEvent(krys::move(type), initializer));
    event->_isTrusted = true;
    return event;
  }

  RefPtr<PointerEvent> PointerEvent::Create(MouseButton button, const MouseEvent &mouseEvent,
                                            web::PointerId pointerId, const DOMString &pointerType) noexcept
  {
    auto type = TypeFromMouseEventType(mouseEvent.Type());
    if (type.View().empty())
    {
      return nullptr;
    }

    return Create(type, button, mouseEvent, pointerId, pointerType);
  }

  Ref<PointerEvent> PointerEvent::Create(const DOMStringAtom &type, MouseButton button,
                                         const MouseEvent &mouseEvent, web::PointerId pointerId,
                                         const DOMString &pointerType) noexcept
  {
    return Create(type, button, mouseEvent, pointerId, pointerType, TypeCanBubble(type),
                  TypeIsCancelable(type));
  }

  Ref<PointerEvent> PointerEvent::Create(const DOMStringAtom &type, MouseButton button,
                                         const MouseEvent &mouseEvent, web::PointerId pointerId,
                                         const DOMString &pointerType, bool canBubble,
                                         bool isCancelable) noexcept
  {
    return AdoptRef(*new PointerEvent(type, button, mouseEvent, pointerId, pointerType, canBubble,
                                      isCancelable, TypeIsComposed(type)));
  }

  Ref<PointerEvent> PointerEvent::Create(DOMStringAtom type, Window *view, web::PointerId pointerId,
                                         const DOMString &pointerType, IsPrimary isPrimary) noexcept
  {
    Ref<PointerEvent> event =
      AdoptRef(*new PointerEvent(krys::move(type), pointerId, pointerType, isPrimary));
    event->SetView(view);
    return event;
  }

  Ref<PointerEvent> PointerEvent::Create(const PlatformTouchEvent &event,
                                         const List<Ref<PointerEvent>> &coalescedEvents,
                                         const List<Ref<PointerEvent>> &predictedEvents, uint32 index,
                                         bool isPrimary, web::FrameView *view,
                                         const PointD &touchDelta) noexcept
  {
    auto type = PointerEventType(event.TouchPoints()[index].GetState());
    return AdoptRef(*new PointerEvent(type, event, coalescedEvents, predictedEvents, TypeCanBubble(type),
                                      TypeIsCancelable(type), index, isPrimary, view, touchDelta));
  }

  Ref<PointerEvent> PointerEvent::Create(const PlatformTouchEvent &event,
                                         const List<Ref<PointerEvent>> &coalescedEvents,
                                         const List<Ref<PointerEvent>> &predictedEvents, bool canBubble,
                                         bool isCancelable, uint32 index, bool isPrimary,
                                         web::FrameView *view, const PointD &touchDelta) noexcept
  {
    auto type = PointerEventType(event.TouchPoints()[index].GetState());
    return AdoptRef(*new PointerEvent(type, event, coalescedEvents, predictedEvents, canBubble, isCancelable,
                                      index, isPrimary, view, touchDelta));
  }

  Ref<PointerEvent> PointerEvent::Create(DOMStringAtom type, const PlatformTouchEvent &event,
                                         const List<Ref<PointerEvent>> &coalescedEvents,
                                         const List<Ref<PointerEvent>> &predictedEvents, bool canBubble,
                                         bool isCancelable, uint32 index, bool isPrimary,
                                         web::FrameView *view, const PointD &touchDelta) noexcept
  {
    return AdoptRef(*new PointerEvent(krys::move(type), event, coalescedEvents, predictedEvents, canBubble,
                                      isCancelable, index, isPrimary, view, touchDelta));
  }

  Ref<PointerEvent> PointerEvent::Create(const DOMStringAtom &type, const PlatformTouchEvent &event,
                                         const List<Ref<PointerEvent>> &coalescedEvents,
                                         const List<Ref<PointerEvent>> &predictedEvents, uint32 index,
                                         bool isPrimary, web::FrameView *view,
                                         const PointD &touchDelta) noexcept
  {
    return AdoptRef(*new PointerEvent(type, event, coalescedEvents, predictedEvents, TypeCanBubble(type),
                                      TypeIsCancelable(type), index, isPrimary, view, touchDelta));
  }

  DOMStringAtom PointerEvent::TypeForTouchPoint(const PlatformTouchPoint &point) noexcept
  {
    return PointerEventType(point.GetState());
  }

  web::PointerId PointerEvent::PointerIdForTouch(const PlatformTouchPoint &point) noexcept
  {
    return TouchMinimumPointerId + point.Id();
  }

  // Calculated in accordance with
  // https://w3c.github.io/pointerevents/#converting-between-tiltx-tilty-and-altitudeangle-azimuthangle.
  auto PointerEvent::AngleFromTilt(int32 tiltX, int32 tiltY) noexcept -> PointerEventAngle
  {
    const double tiltXRadians = tiltX * RadiansPerDegree;
    const double tiltYRadians = tiltY * RadiansPerDegree;
    double azimuthAngle = 0;

    if (!tiltX && tiltY)
    {
      azimuthAngle = tiltY > 0 ? PiOverTwo : 3 * PiOverTwo;
    }
    else if (tiltX && !tiltY)
    {
      azimuthAngle = tiltX < 0 ? std::numbers::pi : azimuthAngle;
    }
    else if (std::abs(tiltX) == 90 || std::abs(tiltY) == 90)
    {
      azimuthAngle = 0;
    }
    else
    {
      azimuthAngle = std::atan2(std::tan(tiltYRadians), std::tan(tiltXRadians));
      azimuthAngle = azimuthAngle < 0 ? azimuthAngle + RadiansPerTurn : azimuthAngle;
    }

    double altitudeAngle = 0;

    if (std::abs(tiltX) == 90 || std::abs(tiltY) == 90)
    {
      altitudeAngle = 0;
    }
    else if (!tiltX)
    {
      altitudeAngle = PiOverTwo - std::abs(tiltYRadians);
    }
    else if (!tiltY)
    {
      altitudeAngle = PiOverTwo - std::abs(tiltXRadians);
    }
    else
    {
      altitudeAngle =
        std::atan(1.0 / std::sqrt(std::pow(std::tan(tiltXRadians), 2) + std::pow(std::tan(tiltYRadians), 2)));
    }

    return {altitudeAngle, azimuthAngle};
  }

  // This algorithm is equivalent to the one provided in
  // https://w3c.github.io/pointerevents/#converting-between-tiltx-tilty-and-altitudeangle-azimuthangle.
  auto PointerEvent::TiltFromAngle(double altitudeAngle, double azimuthAngle) noexcept -> PointerEventTilt
  {
    double tiltXRadians = AreEssentiallyEqual(altitudeAngle, 0.0)
                            ? std::cos(azimuthAngle) * PiOverTwo
                            : std::atan(std::cos(azimuthAngle) / std::tan(altitudeAngle));
    double tiltYRadians = AreEssentiallyEqual(altitudeAngle, 0.0)
                            ? std::sin(azimuthAngle) * PiOverTwo
                            : std::atan(std::sin(azimuthAngle) / std::tan(altitudeAngle));

    return {static_cast<int32>(std::round(tiltXRadians * DegreesPerRadian)),
            static_cast<int32>(std::round(tiltYRadians * DegreesPerRadian))};
  }

  PointerEvent::PointerEvent(const DOMStringAtom &type, const PointerEventInit &initializer) noexcept
      : MouseEvent(type, initializer), _pointerId(static_cast<web::PointerId>(initializer.PointerId)),
        _width(initializer.Width), _height(initializer.Height), _pressure(initializer.Pressure),
        _tangentialPressure(initializer.TangentialPressure), _tiltX(initializer.TiltX),
        _tiltY(initializer.TiltY), _twist(initializer.Twist), _altitudeAngle(initializer.AltitudeAngle),
        _azimuthAngle(initializer.AzimuthAngle), _pointerType(initializer.PointerType),
        _isPrimary(initializer.IsPrimary),
        _fractionalCoordinatesAllowed(FractionalCoordinatesAllowedForType(type)),
        _coalescedEvents(initializer.CoalescedEvents), _predictedEvents(initializer.PredictedEvents)
  {
    // Whichever pair is left at its defaults follows the other; both are kept if both are given.
    // https://w3c.github.io/pointerevents/#converting-between-tiltx-tilty-and-altitudeangle-azimuthangle
    bool tiltGiven = initializer.TiltX || initializer.TiltY;
    bool anglesGiven = initializer.AzimuthAngle || initializer.AltitudeAngle != PiOverTwo;

    if (tiltGiven && !anglesGiven)
    {
      PointerEventAngle angle = AngleFromTilt(_tiltX, _tiltY);
      _altitudeAngle = angle.AltitudeAngle;
      _azimuthAngle = angle.AzimuthAngle;
    }
    else if (anglesGiven && !tiltGiven)
    {
      PointerEventTilt tilt = TiltFromAngle(_altitudeAngle, _azimuthAngle);
      _tiltX = tilt.TiltX;
      _tiltY = tilt.TiltY;
    }
  }

  PointerEvent::PointerEvent(const DOMStringAtom &type, MouseButton button, const MouseEvent &mouseEvent,
                             web::PointerId pointerId, const DOMString &pointerType, bool canBubble,
                             bool isCancelable, bool isComposed) noexcept
      : MouseEvent(type, TrustedInit(canBubble, isCancelable, isComposed, mouseEvent.Detail()),
                   mouseEvent.FrameView(), mouseEvent.ScreenLocation(), mouseEvent.WindowLocation(),
                   mouseEvent.MovementX(), mouseEvent.MovementY(), mouseEvent.ModifierKeys(), button,
                   mouseEvent.Buttons(), mouseEvent.RelatedTarget().get(), 0,
                   mouseEvent.GetSyntheticClickType(), {}, {},
                   mouseEvent.IsSimulated() ? Simulated::Yes : Simulated::No),
        _pointerId(pointerId),
        // MouseEvent is a misnomer in this context, and can represent events from a pressure sensitive input
        // device if the pointer type is "Pen" or "Touch". If it does represent a pressure sensitive input
        // device, we consult MouseEvent::force() for the event pressure, else we fall back to spec defaults.
        _pressure(pointerType != pointer_events::MousePointerEventType()
                    ? static_cast<float>(std::clamp(mouseEvent.Force(), 0., 1.))
                    : PressureForPressureInsensitiveInputDevices(Buttons())),
        _pointerType(pointerType), _isPrimary(true),
        _fractionalCoordinatesAllowed(FractionalCoordinatesAllowedForType(type)),
        _coalescedEvents(CreateCoalescedPointerEvents(type, button, mouseEvent, pointerId, pointerType)),
        _predictedEvents(CreatePredictedPointerEvents(type, button, mouseEvent, pointerId, pointerType))
  {
    _createTime = mouseEvent.TimeStamp();
    _isTrusted = mouseEvent.IsTrusted();

    // A pen reports how it's held.
    if (pointerType == pointer_events::PenPointerEventType())
    {
      auto &orientation = mouseEvent.GetPenOrientation();
      _tiltX = orientation.TiltX;
      _tiltY = orientation.TiltY;
      _twist = orientation.Twist;
      PointerEventAngle angle = AngleFromTilt(_tiltX, _tiltY);
      _altitudeAngle = angle.AltitudeAngle;
      _azimuthAngle = angle.AzimuthAngle;
    }
  }

  PointerEvent::PointerEvent(const DOMStringAtom &type, web::PointerId pointerId, DOMString pointerType,
                             IsPrimary isPrimary) noexcept
      : MouseEvent(type, TrustedInit(TypeCanBubble(type), TypeIsCancelable(type), TypeIsComposed(type)),
                   nullptr, {}, {}, 0, 0, {}, ButtonForType(type), ButtonsForType(type), nullptr, 0,
                   SyntheticClickType::NoTap, {}, {}, Simulated::No),
        _pointerId(pointerId),
        // FIXME: This may be wrong because we can create an event from a pressure sensitive device.
        // We don't have a backing MouseEvent to consult pressure/force information from, though, so let's do
        // the next best thing.
        _pressure(PressureForPressureInsensitiveInputDevices(Buttons())),
        _pointerType(krys::move(pointerType)), _isPrimary(isPrimary == IsPrimary::Yes),
        _fractionalCoordinatesAllowed(FractionalCoordinatesAllowedForType(type))
  {
  }

  PointerEvent::PointerEvent(const DOMStringAtom &type, const PlatformTouchEvent &event,
                             const List<Ref<PointerEvent>> &coalescedEvents,
                             const List<Ref<PointerEvent>> &predictedEvents, bool canBubble,
                             bool isCancelable, uint32 index, bool isPrimary, web::FrameView *view,
                             const PointD &touchDelta) noexcept
      : MouseEvent(type, TrustedInit(canBubble, isCancelable, TypeIsComposed(type)), view,
                   event.TouchPoints()[index].ScreenPos(), event.TouchPoints()[index].Pos(), touchDelta.X(),
                   touchDelta.Y(), event.Modifiers(), ButtonForType(type), ButtonsForType(type), nullptr, 0,
                   SyntheticClickType::NoTap, {}, {}, Simulated::No),
        _pointerId(PointerIdForTouch(event.TouchPoints()[index])),
        _width(TouchContactLength(event.TouchPoints()[index].Radius().Width())),
        _height(TouchContactLength(event.TouchPoints()[index].Radius().Height())),
        _pressure(event.TouchPoints()[index].Force() > 0
                    ? event.TouchPoints()[index].Force()
                    : PressureForPressureInsensitiveInputDevices(ButtonsForType(type))),
        _twist(static_cast<int32>(event.TouchPoints()[index].Twist())),
        _pointerType(pointer_events::TouchPointerEventType()), _isPrimary(isPrimary),
        _fractionalCoordinatesAllowed(FractionalCoordinatesAllowedForType(type)),
        _coalescedEvents(coalescedEvents), _predictedEvents(predictedEvents)
  {
    _createTime = event.Timestamp();
  }

  PointerEvent::~PointerEvent() noexcept = default;

  List<Ref<PointerEvent>> PointerEvent::GetCoalescedEvents() const noexcept
  {
#if KRYS_DEBUG
    if (IsTrusted())
    {
      if (Type() == event_names::PointerMove || Type() == event_names::PointerRawUpdate)
      {
        krys_debug_assert_msg(
          !_coalescedEvents.empty(),
          "Trusted pointermove and pointerrawupdate events must have more than zero coalesced events.");
      }
      else
      {
        krys_debug_assert_msg(
          _coalescedEvents.empty(),
          "Trusted events other than pointermove and pointerrawupdate must have zero coalesced events.");
      }

      auto timestampsAreMonotonicallyIncreasing =
        std::ranges::is_sorted(_coalescedEvents, [](const auto &previousEvent, const auto &newEvent)
                               { return previousEvent->TimeStamp() <= newEvent->TimeStamp(); });
      krys_debug_assert_msg(timestampsAreMonotonicallyIncreasing,
                            "Coalesced event timestamps are not monotonically increasing.");

      auto canBubbleOrIsCancelable = std::ranges::any_of(_coalescedEvents, [](const auto &event)
                                                         { return event->Bubbles() || event->Cancelable(); });
      krys_debug_assert_msg(!canBubbleOrIsCancelable,
                            "Coalesced events must not bubble and must not be cancelable.");
    }
#endif

    return _coalescedEvents;
  }

  List<Ref<PointerEvent>> PointerEvent::GetPredictedEvents() const noexcept
  {
#if KRYS_DEBUG
    if (IsTrusted())
    {
      if (Type() != event_names::PointerMove)
      {
        krys_debug_assert_msg(_predictedEvents.empty(),
                              "Trusted non-pointermove events must have zero predicted events.");
      }

      auto timestampsAreMonotonicallyIncreasing =
        std::ranges::is_sorted(_predictedEvents, [](const auto &previousEvent, const auto &newEvent)
                               { return previousEvent->TimeStamp() <= newEvent->TimeStamp(); });
      krys_debug_assert_msg(timestampsAreMonotonicallyIncreasing,
                            "Predicted event timestamps are not monotonically increasing.");

      if (!_predictedEvents.empty())
      {
        krys_debug_assert_msg(
          _predictedEvents.front()->TimeStamp() >= TimeStamp(),
          "Predicted events must have a timestamp greater than or equal to the dispatched event.");
      }

      auto canBubbleOrIsCancelable = std::ranges::any_of(_predictedEvents, [](const auto &event)
                                                         { return event->Bubbles() || event->Cancelable(); });
      krys_debug_assert_msg(!canBubbleOrIsCancelable,
                            "Predicted events must not bubble and must not be cancelable.");
    }
#endif

    return _predictedEvents;
  }

  void PointerEvent::ReceivedTarget() noexcept
  {
    MouseEvent::ReceivedTarget();

    if (!IsTrusted())
    {
      return;
    }

    for (auto &coalescedEvent : _coalescedEvents)
    {
      coalescedEvent->SetTarget(Target());
    }

    for (auto &predictedEvent : _predictedEvents)
    {
      predictedEvent->SetTarget(Target());
    }
  }

  bool PointerEvent::FractionalCoordinatesAllowedForType(const DOMStringAtom &type) noexcept
  {
    return !(type == event_names::AuxClick || type == event_names::Click || type == event_names::ContextMenu);
  }

  double PointerEvent::AdjustedCoordinateForType(double coordinate) const noexcept
  {
    if (FractionalCoordinatesAllowed())
    {
      return coordinate;
    }

    return std::floor(coordinate);
  }

  double PointerEvent::OffsetX() const noexcept
  {
    if (IsSimulated())
    {
      return 0;
    }
    if (!HasCachedRelativePosition())
    {
      ComputeRelativePosition();
    }

    return AdjustedCoordinateForType(OffsetLocation().X());
  }

  double PointerEvent::OffsetY() const noexcept
  {
    if (IsSimulated())
    {
      return 0;
    }
    if (!HasCachedRelativePosition())
    {
      ComputeRelativePosition();
    }

    return AdjustedCoordinateForType(OffsetLocation().Y());
  }
}

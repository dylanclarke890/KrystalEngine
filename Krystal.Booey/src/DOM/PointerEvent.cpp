#include "Krystal.Booey/DOM/PointerEvent.hpp"
#include "Krystal.Booey/Core/MathExtras.hpp"
#include "Krystal.Booey/Platform/PlatformTouchEvent.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Debug.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo::dom
{
  namespace
  {
    constexpr double PiOverTwo = std::numbers::pi / 2;
    constexpr double RadiansPerDegree = std::numbers::pi / 180;
    constexpr double DegreesPerRadian = 180 / std::numbers::pi;
    constexpr double RadiansPerTurn = 2 * std::numbers::pi;

    // The touches' pointer ids follow the mouse's and the pen's: the host numbers its touches from 0.
    constexpr PointerID TouchMinimumPointerID = PenPointerID + 1;

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
        case PlatformTouchPoint::TouchPressed:    return DOMStringAtom(EventNames::PointerDown);
        case PlatformTouchPoint::TouchMoved:      return DOMStringAtom(EventNames::PointerMove);
        case PlatformTouchPoint::TouchStationary: return DOMStringAtom(EventNames::PointerMove);
        case PlatformTouchPoint::TouchReleased:   return DOMStringAtom(EventNames::PointerUp);
        case PlatformTouchPoint::TouchCancelled:  return DOMStringAtom(EventNames::PointerCancel);
        case PlatformTouchPoint::TouchStateEnd:   break;
      }
      krys_debug_assert_msg(false, "Not reached.");
      return DOMStringAtom::Null();
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

    KRYS_NODISCARD List<Ref<PointerEvent>>
      CreateCoalescedPointerEvents(DOMStringAtom type, MouseButton button, const MouseEvent &mouseEvent,
                                   PointerID pointerId, const DOMString &pointerType) noexcept
    {
      if (type != EventNames::PointerMove)
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

    KRYS_NODISCARD List<Ref<PointerEvent>>
      CreatePredictedPointerEvents(DOMStringAtom type, MouseButton button, const MouseEvent &mouseEvent,
                                   PointerID pointerId, const DOMString &pointerType) noexcept
    {
      if (type != EventNames::PointerMove)
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

  DOMStringAtom PointerEvent::TypeFromMouseEventType(DOMStringAtom mouseEventType) noexcept
  {
    if (mouseEventType == EventNames::AuxClick)
    {
      return DOMStringAtom(EventNames::AuxClick);
    }
    if (mouseEventType == EventNames::Click)
    {
      return DOMStringAtom(EventNames::Click);
    }
    if (mouseEventType == EventNames::ContextMenu)
    {
      return DOMStringAtom(EventNames::ContextMenu);
    }
    if (mouseEventType == EventNames::MouseDown)
    {
      return DOMStringAtom(EventNames::PointerDown);
    }
    if (mouseEventType == EventNames::MouseOver)
    {
      return DOMStringAtom(EventNames::PointerOver);
    }
    if (mouseEventType == EventNames::MouseEnter)
    {
      return DOMStringAtom(EventNames::PointerEnter);
    }
    if (mouseEventType == EventNames::MouseMove)
    {
      return DOMStringAtom(EventNames::PointerMove);
    }
    if (mouseEventType == EventNames::MouseLeave)
    {
      return DOMStringAtom(EventNames::PointerLeave);
    }
    if (mouseEventType == EventNames::MouseOut)
    {
      return DOMStringAtom(EventNames::PointerOut);
    }
    if (mouseEventType == EventNames::MouseUp)
    {
      return DOMStringAtom(EventNames::PointerUp);
    }

    return DOMStringAtom::Null();
  }

  bool PointerEvent::TypeRequiresResolvedButton(DOMStringAtom type) noexcept
  {
    return type == EventNames::PointerUp || type == EventNames::PointerDown || type == EventNames::Click
           || type == EventNames::AuxClick || type == EventNames::ContextMenu;
  }

  Ref<PointerEvent> PointerEvent::CreateForPointerCapture(DOMStringAtom type, PointerID pointerId,
                                                          bool isPrimary,
                                                          const DOMString &pointerType) noexcept
  {
    PointerEventInit initializer;
    initializer.Bubbles = true;
    initializer.PointerId = static_cast<int32>(pointerId);
    initializer.IsPrimary = isPrimary;
    initializer.PointerType = pointerType;
    initializer.Composed = true;
    Ref<PointerEvent> event = AdoptRef(*new PointerEvent(type, initializer));
    event->_isTrusted = true;
    return event;
  }

  RefPtr<PointerEvent> PointerEvent::Create(MouseButton button, const MouseEvent &mouseEvent,
                                            PointerID pointerId, const DOMString &pointerType) noexcept
  {
    auto type = TypeFromMouseEventType(mouseEvent.Type());
    if (type.View().empty())
    {
      return nullptr;
    }

    return Create(type, button, mouseEvent, pointerId, pointerType);
  }

  Ref<PointerEvent> PointerEvent::Create(DOMStringAtom type, MouseButton button, const MouseEvent &mouseEvent,
                                         PointerID pointerId, const DOMString &pointerType) noexcept
  {
    return Create(type, button, mouseEvent, pointerId, pointerType, TypeCanBubble(type),
                  TypeIsCancelable(type));
  }

  Ref<PointerEvent> PointerEvent::Create(DOMStringAtom type, MouseButton button, const MouseEvent &mouseEvent,
                                         PointerID pointerId, const DOMString &pointerType, bool canBubble,
                                         bool isCancelable) noexcept
  {
    return AdoptRef(*new PointerEvent(type, button, mouseEvent, pointerId, pointerType, canBubble,
                                      isCancelable, TypeIsComposed(type)));
  }

  Ref<PointerEvent> PointerEvent::Create(DOMStringAtom type, PointerID pointerId,
                                         const DOMString &pointerType, IsPrimary isPrimary) noexcept
  {
    return AdoptRef(*new PointerEvent(type, pointerId, pointerType, isPrimary));
  }

  Ref<PointerEvent> PointerEvent::Create(const PlatformTouchEvent &event,
                                         const List<Ref<PointerEvent>> &coalescedEvents,
                                         const List<Ref<PointerEvent>> &predictedEvents, uint32 index,
                                         bool isPrimary, LocalFrameView *view,
                                         const DoublePoint &touchDelta) noexcept
  {
    auto type = PointerEventType(event.TouchPoints()[index].GetState());
    return AdoptRef(*new PointerEvent(type, event, coalescedEvents, predictedEvents, TypeCanBubble(type),
                                      TypeIsCancelable(type), index, isPrimary, view, touchDelta));
  }

  Ref<PointerEvent> PointerEvent::Create(const PlatformTouchEvent &event,
                                         const List<Ref<PointerEvent>> &coalescedEvents,
                                         const List<Ref<PointerEvent>> &predictedEvents, bool canBubble,
                                         bool isCancelable, uint32 index, bool isPrimary,
                                         LocalFrameView *view, const DoublePoint &touchDelta) noexcept
  {
    auto type = PointerEventType(event.TouchPoints()[index].GetState());
    return AdoptRef(*new PointerEvent(type, event, coalescedEvents, predictedEvents, canBubble, isCancelable,
                                      index, isPrimary, view, touchDelta));
  }

  Ref<PointerEvent> PointerEvent::Create(DOMStringAtom type, const PlatformTouchEvent &event,
                                         const List<Ref<PointerEvent>> &coalescedEvents,
                                         const List<Ref<PointerEvent>> &predictedEvents, uint32 index,
                                         bool isPrimary, LocalFrameView *view,
                                         const DoublePoint &touchDelta) noexcept
  {
    return AdoptRef(*new PointerEvent(type, event, coalescedEvents, predictedEvents, TypeCanBubble(type),
                                      TypeIsCancelable(type), index, isPrimary, view, touchDelta));
  }

  PointerID PointerEvent::PointerIdForTouch(const PlatformTouchPoint &point) noexcept
  {
    return TouchMinimumPointerID + point.Id();
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

  PointerEvent::PointerEvent(DOMStringAtom type, const PointerEventInit &initializer) noexcept
      : MouseEvent(type, initializer), _pointerId(static_cast<PointerID>(initializer.PointerId)),
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

  PointerEvent::PointerEvent(DOMStringAtom type, MouseButton button, const MouseEvent &mouseEvent,
                             PointerID pointerId, const DOMString &pointerType, bool canBubble,
                             bool isCancelable, bool isComposed) noexcept
      : MouseEvent(type, TrustedInit(canBubble, isCancelable, isComposed, mouseEvent.Detail()),
                   mouseEvent.View(), mouseEvent.ScreenLocation(), mouseEvent.WindowLocation(),
                   mouseEvent.MovementX(), mouseEvent.MovementY(), mouseEvent.ModifierKeys(), button,
                   mouseEvent.Buttons(), mouseEvent.RelatedTarget().get(), 0,
                   mouseEvent.GetSyntheticClickType(), {}, {},
                   mouseEvent.IsSimulated() ? Simulated::Yes : Simulated::No),
        _pointerId(pointerId),
        // MouseEvent is a misnomer in this context, and can represent events from a pressure sensitive input
        // device if the pointer type is "Pen" or "Touch". If it does represent a pressure sensitive input
        // device, we consult MouseEvent::force() for the event pressure, else we fall back to spec defaults.
        _pressure(pointerType != MousePointerEventType()
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
    if (pointerType == PenPointerEventType())
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

  PointerEvent::PointerEvent(DOMStringAtom type, PointerID pointerId, const DOMString &pointerType,
                             IsPrimary isPrimary) noexcept
      : MouseEvent(type, TrustedInit(TypeCanBubble(type), TypeIsCancelable(type), TypeIsComposed(type)),
                   nullptr, {}, {}, 0, 0, {}, ButtonForType(type), ButtonsForType(type), nullptr, 0,
                   SyntheticClickType::NoTap, {}, {}, Simulated::No),
        _pointerId(pointerId),
        // FIXME: This may be wrong because we can create an event from a pressure sensitive device.
        // We don't have a backing MouseEvent to consult pressure/force information from, though, so let's do
        // the next best thing.
        _pressure(PressureForPressureInsensitiveInputDevices(Buttons())), _pointerType(pointerType),
        _isPrimary(isPrimary == IsPrimary::Yes),
        _fractionalCoordinatesAllowed(FractionalCoordinatesAllowedForType(type))
  {
  }

  PointerEvent::PointerEvent(DOMStringAtom type, const PlatformTouchEvent &event,
                             const List<Ref<PointerEvent>> &coalescedEvents,
                             const List<Ref<PointerEvent>> &predictedEvents, bool canBubble,
                             bool isCancelable, uint32 index, bool isPrimary, LocalFrameView *view,
                             const DoublePoint &touchDelta) noexcept
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
        _twist(static_cast<int32>(event.TouchPoints()[index].Twist())), _pointerType(TouchPointerEventType()),
        _isPrimary(isPrimary), _fractionalCoordinatesAllowed(FractionalCoordinatesAllowedForType(type)),
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
      if (Type() == EventNames::PointerMove)
      {
        krys_debug_assert_msg(!_coalescedEvents.empty(),
                              "Trusted pointermove events must have more than zero coalesced events.");
      }
      else
      {
        krys_debug_assert_msg(_coalescedEvents.empty(),
                              "Trusted non-pointermove events must have zero coalesced events.");
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
      if (Type() != EventNames::PointerMove)
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

  bool PointerEvent::FractionalCoordinatesAllowedForType(DOMStringAtom type) noexcept
  {
    return !(type == EventNames::AuxClick || type == EventNames::Click || type == EventNames::ContextMenu);
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

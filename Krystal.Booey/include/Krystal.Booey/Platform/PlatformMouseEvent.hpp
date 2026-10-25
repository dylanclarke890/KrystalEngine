#pragma once

#include "Krystal.Booey/DOM/PointerEventTypeNames.hpp"
#include "Krystal.Booey/Graphics/DoublePoint.hpp"
#include "Krystal.Booey/Page/MouseEventTypes.hpp"
#include "Krystal.Booey/Platform/PlatformEvent.hpp"
#include "Krystal.Booey/Platform/PointerID.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief A mouse move, press or release from the host (or a pen's, or a tap's standing in for a click).
  class PlatformMouseEvent : public PlatformEvent
  {
  protected:
    MouseButton _button {MouseButton::None};
    SyntheticClickType _syntheticClickType {SyntheticClickType::NoTap};
    DoublePoint _position;
    DoublePoint _globalPosition;
    DoublePoint _movementDelta;
    double _force {0};
    PenOrientation _penOrientation;
    PointerID _pointerId {MousePointerID};
    dom::DOMString _pointerType {dom::MousePointerEventType()};
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
    PlatformMouseEvent(const DoublePoint &position, const DoublePoint &globalPosition, MouseButton button,
                       PlatformEvent::Type type, int32 clickCount, EnumSet<PlatformEvent::Modifier> modifiers,
                       MonotonicTime::duration timestamp, uint16 buttons = 0,
                       const DoublePoint &movementDelta = {}, double force = 0,
                       SyntheticClickType syntheticClickType = SyntheticClickType::NoTap,
                       PointerID pointerId = MousePointerID,
                       const dom::DOMString &pointerType = dom::MousePointerEventType()) noexcept
        : PlatformEvent(type, modifiers, timestamp), _button(button), _syntheticClickType(syntheticClickType),
          _position(position), _globalPosition(globalPosition), _movementDelta(movementDelta), _force(force),
          _pointerId(pointerId), _pointerType(pointerType), _clickCount(clickCount), _buttons(buttons)
    {
    }

    // This position is relative to the view. Use the view's windowToContents() to convert it to into the
    // contents of a given view.
    KRYS_NODISCARD const DoublePoint &Position() const noexcept
    {
      return _position;
    }

    KRYS_NODISCARD const DoublePoint &GlobalPosition() const noexcept
    {
      return _globalPosition;
    }

    KRYS_NODISCARD const DoublePoint &MovementDelta() const noexcept
    {
      return _movementDelta;
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

    KRYS_NODISCARD PointerID PointerId() const noexcept
    {
      return _pointerId;
    }

    KRYS_NODISCARD const dom::DOMString &PointerType() const noexcept
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

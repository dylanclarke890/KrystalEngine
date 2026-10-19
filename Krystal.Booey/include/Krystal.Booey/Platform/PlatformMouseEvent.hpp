#pragma once

#include "Krystal.Booey/Graphics/DoublePoint.hpp"
#include "Krystal.Booey/Page/MouseEventTypes.hpp"
#include "Krystal.Booey/Platform/PlatformEvent.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief A mouse move, press or release from the host.
  // FIXME: Force, synthetic clicks, pointer ids and types, and coalesced and predicted events come with
  // pointer events.
  class PlatformMouseEvent : public PlatformEvent
  {
  protected:
    MouseButton _button {MouseButton::None};
    DoublePoint _position;
    DoublePoint _globalPosition;
    DoublePoint _movementDelta;
    int32 _clickCount {0};
    uint16 _buttons {0};

  public:
    PlatformMouseEvent() noexcept : PlatformEvent(Type::MouseMoved)
    {
    }

    /// @param position The point in the view's coordinates.
    /// @param globalPosition The point in the screen's coordinates.
    /// @param buttons The buttons held, as the DOM's `buttons` bit set.
    PlatformMouseEvent(const DoublePoint &position, const DoublePoint &globalPosition, MouseButton button,
                       PlatformEvent::Type type, int32 clickCount, EnumSet<PlatformEvent::Modifier> modifiers,
                       MonotonicTime::duration timestamp, uint16 buttons = 0,
                       const DoublePoint &movementDelta = {}) noexcept
        : PlatformEvent(type, modifiers, timestamp), _button(button), _position(position),
          _globalPosition(globalPosition), _movementDelta(movementDelta), _clickCount(clickCount),
          _buttons(buttons)
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
  };
}

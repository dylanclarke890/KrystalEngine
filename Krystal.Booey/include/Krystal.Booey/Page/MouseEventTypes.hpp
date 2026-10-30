#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  static constexpr double ForceAtClick = 1;
  static constexpr double ForceAtForceClick = 2;

  /// @brief Whether a mouse event stands in for a tap, and with how many fingers.
  /// @brief How a pen is held: its tilt along each axis and its twist about its own axis, in degrees.
  /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-tiltx
  struct PenOrientation
  {
    int32 TiltX {0};
    int32 TiltY {0};
    int32 Twist {0};
  };

  enum class SyntheticClickType : uint8
  {
    NoTap,
    OneFingerTap,
    TwoFingerTap
  };

  // These button numbers match the ones used in the DOM API, 0 through 2, except for None and Other which
  // aren't specified. We reserve -2 for the former and -1 to represent pointer events that indicate that the
  // pressed mouse button hasn't changed since the last event, as specified in the DOM API for Pointer Events.
  // https://w3c.github.io/uievents/#dom-mouseevent-button
  // https://w3c.github.io/pointerevents/#the-button-property
  enum class MouseButton : int8
  {
    None = -2,
    PointerHasNotChanged,
    Left,
    Middle,
    Right,
    Back,
    Forward,
    Other
  };

  /// @brief The button a DOM `button` value names, or `Other` if it names none.
  KRYS_NODISCARD constexpr MouseButton ButtonFromShort(int16 buttonValue) noexcept
  {
    if (buttonValue >= static_cast<int16>(MouseButton::None)
        && buttonValue < static_cast<int16>(MouseButton::Other))
    {
      return static_cast<MouseButton>(buttonValue);
    }
    return MouseButton::Other;
  }
}

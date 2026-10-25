#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.PAL/KeyModifiers.hpp"

namespace krys::pal
{
  /// @brief The kind of device behind a pointer that isn't the mouse.
  enum class PointerType : uint8
  {
    Pen,
    Touch,
  };

  /// @brief What happened to a pen or a touch.
  enum class PointerAction : uint8
  {
    /// @brief A pen came into range over the window, hovering.
    Entered,
    /// @brief The pointer touched the surface.
    Pressed,
    /// @brief The pointer moved (hovering or touching), or its pressure, tilt or buttons changed.
    Moved,
    /// @brief The pointer left the surface.
    Released,
    /// @brief A pen went out of range, or off the window.
    Left,
    /// @brief The system took the pointer away (for a system gesture, or as the window lost it).
    Cancelled,
  };

  /// @brief A pen's or a touch's input, as the window reports it.
  struct PointerInput
  {
    /// @brief Identifies the pointer while it's in range: the pointers in range have distinct ids, which a
    /// later one may reuse.
    uint32 Id {0};
    PointerType Type {PointerType::Touch};
    PointerAction Action {PointerAction::Moved};
    /// @brief The pointer's position in client space.
    float ClientX {0};
    float ClientY {0};
    /// @brief The pointer's position on the screen.
    float ScreenX {0};
    float ScreenY {0};
    /// @brief Whether the pointer touches the surface (false for a hovering pen).
    bool InContact {false};
    /// @brief Whether it's the primary pointer: the first touch of several, or the pen.
    bool IsPrimary {false};
    /// @brief The pressure, from 0 to 1, if the device reports it.
    Maybe<float> Pressure;
    /// @brief The pen's tilt towards the right and towards the user, in degrees from -90 to 90.
    float TiltX {0};
    float TiltY {0};
    /// @brief The pen's clockwise rotation about its own axis, in degrees from 0 to 359.
    float Twist {0};
    /// @brief The touch's contact size in client space, or 0 if the device doesn't report it.
    float Width {0};
    float Height {0};
    /// @brief Whether the pen's barrel button is held.
    bool BarrelButton {false};
    /// @brief Whether the pen is the eraser end (inverted, or its eraser button held).
    bool Eraser {false};
    /// @brief The modifier keys held.
    KeyModifiers Modifiers;
  };

  /// @brief The pointing devices the system has.
  struct PointingDevices
  {
    bool Mouse {true};
    bool Pen {false};
    bool Touch {false};
    /// @brief How many touches the touch screens take at once; 0 without one.
    uint32 MaxTouchPoints {0};
  };
}

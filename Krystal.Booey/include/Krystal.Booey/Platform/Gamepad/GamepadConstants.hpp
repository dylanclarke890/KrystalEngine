#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief Buttons in the "standard" gamepad layout in the Web Gamepad spec.
  /// @see https://w3c.github.io/gamepad/#dfn-standard-gamepad
  enum class GamepadButtonRole : uint8
  {
    RightClusterBottom = 0,
    RightClusterRight = 1,
    RightClusterLeft = 2,
    RightClusterTop = 3,
    LeftShoulderFront = 4,
    RightShoulderFront = 5,
    LeftShoulderBack = 6,
    RightShoulderBack = 7,
    CenterClusterLeft = 8,
    CenterClusterRight = 9,
    LeftStick = 10,
    RightStick = 11,
    LeftClusterTop = 12,
    LeftClusterBottom = 13,
    LeftClusterLeft = 14,
    LeftClusterRight = 15,
    CenterClusterCenter = 16,
    Nonstandard1 = 17,
    Nonstandard2 = 18,
  };

  inline constexpr GamepadButtonRole MaximumGamepadButton = GamepadButtonRole::CenterClusterCenter;
  inline constexpr size_t NumberOfStandardGamepadButtonsWithoutHomeButton =
    static_cast<size_t>(MaximumGamepadButton);
  inline constexpr size_t NumberOfStandardGamepadButtonsWithHomeButton =
    NumberOfStandardGamepadButtonsWithoutHomeButton + 1;

  /// @brief The mapping of a pad whose buttons and axes follow the standard layout.
  /// @see https://w3c.github.io/gamepad/#dom-gamepadmappingtype-standard
  inline constexpr utf8_stringview StandardGamepadMapping = u8"standard";
}

#pragma once

#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::pal
{
  /// @brief The state of a button during an interaction.
  enum class MouseButtonState : uint8
  {
    Pressed,
    Held,
    Released,
  };

  /// @brief Supported mouse buttons.
  enum class MouseButton : uint8
  {
    Left,
    Right,
    Middle,
    Thumb1,
    Thumb2,
    UNKNOWN,
  };

  consteval MouseButton MaxEnumValue(MouseButton) noexcept
  {
    return MouseButton::UNKNOWN;
  }
}

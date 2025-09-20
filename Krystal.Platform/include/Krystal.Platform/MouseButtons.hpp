#pragma once

#include "Krystal.Lib/Types.hpp"
#include "Krystal.Lib/Macros.hpp"

namespace Krys::Platform
{
  /// @brief The state of a button during an interaction.
  enum class MouseButtonState : uint8
  {
    Pressed,
    Held,
    Released
  };

  /// @brief Supported mouse buttons.
  enum class MouseButton : uint16
  {
    None = 0,
    Left = 1,
    Right = 2,
    Middle = 4,
    Thumb1 = 8,
    Thumb2 = 16,
    UNKNOWN = 32
  };

  ENUM_BITWISE_OPERATORS(MouseButton)
}
#pragma once

#include "Krystal.Core/Core.hpp"

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
    LEFT = 1,
    RIGHT = 2,
    MIDDLE = 4,
    THUMB_1 = 8,
    THUMB_2 = 16,
    UNKNOWN = 32
  };

  ENUM_BITWISE_OPERATORS(MouseButton)
}
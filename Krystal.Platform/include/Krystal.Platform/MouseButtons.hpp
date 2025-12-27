#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

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

  KRYS_ENUM_FLAG_OPERATORS(MouseButton)
}
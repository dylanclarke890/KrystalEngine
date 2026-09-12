#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::pal
{
  /// @brief The state of a button during an interaction.
  enum class MouseButtonState : uint8
  {
    Pressed,
    Held,
    Released
  };

  /// @brief Supported mouse buttons.
  enum class MouseButton : uint8
  {
    None = 0,
    Left = 1 << 0,
    Right = 1 << 1,
    Middle = 1 << 2,
    Thumb1 = 1 << 3,
    Thumb2 = 1 << 4,
    UNKNOWN = 1 << 5
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::pal::MouseButton, 7uz)

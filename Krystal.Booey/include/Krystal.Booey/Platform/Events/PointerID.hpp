#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief Identifies a pointer: the mouse, a pen or a touch.
  using PointerID = uint32;

  /// @brief The mouse's pointer.
  static constexpr PointerID MousePointerID = 1;

  /// @brief The pen's pointer.
  static constexpr PointerID PenPointerID = 2;
}

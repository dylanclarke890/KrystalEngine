#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys
{
  /// @brief How a gradient carries on past its first and last stops.
  enum class GradientSpreadMethod : uint8
  {
    /// @brief The first and last colours carry on.
    Pad,
    /// @brief The gradient repeats, every other repetition reversed.
    Reflect,
    /// @brief The gradient repeats.
    Repeat
  };
}

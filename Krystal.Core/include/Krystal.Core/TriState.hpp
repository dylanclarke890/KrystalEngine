#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys
{
  /// @brief A boolean that can also be neither: false, true, or indeterminate (some of both).
  enum class TriState : uint8
  {
    False,
    True,
    Indeterminate,
  };
}

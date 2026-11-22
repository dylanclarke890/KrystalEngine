#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys
{
  /// @brief Whether two pointers are equal or point to equal values.
  template <typename T>
  KRYS_NODISCARD constexpr bool ArePointingToEqualData(const T &a, const T &b) noexcept
  {
    return a == b || (a && b && *a == *b);
  }
}

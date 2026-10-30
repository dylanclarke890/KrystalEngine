#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief Narrows a `double` to a `float`, making the loss of precision explicit at the call site.
  KRYS_NODISCARD constexpr float NarrowPrecisionToFloat(double number) noexcept
  {
    return static_cast<float>(number);
  }

  // Only `double` can be narrowed; anything else is a mistake.
  template <typename T>
  constexpr float NarrowPrecisionToFloat(T) noexcept = delete;
}

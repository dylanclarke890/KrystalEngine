#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include <utility>

namespace Krys
{
  /// @brief Gets the underlying integer value of an enumeration type.
  template <typename T>
  KRYS_NODISCARD constexpr auto ToUnderlying(T value) noexcept
  {
    return std::to_underlying<T>(value);
  }
}
#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include <utility>

namespace Krys
{
  /// @brief Performs a static cast if it is safe to do so without loss of information; otherwise, it
  /// performs a forward of the original value, allowing the compiler to handle any necessary conversions.
  template <typename TTo, bool RequireAligned = false, typename TFrom>
  constexpr decltype(auto) LosslessStaticCast(TFrom &&from)
  {
    using To = remove_cvref_t<TTo>;
    using From = remove_cvref_t<TFrom>;
    if constexpr (sizeof(To) == sizeof(TFrom) && (alignof(To) == alignof(TFrom) || !RequireAligned))
    {
      // safe to static cast
      return static_cast<To>(from);
    }
    else
    {
      // let the compiler warn of weird conversions
      return std::forward<From>(from);
    }
  }
}

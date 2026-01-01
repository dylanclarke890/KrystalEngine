#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"

namespace Krys
{
  /// @brief Checks whether the given encoding type returns a maybe-replacement range of code units.
  /// @tparam T The type to check.
  /// @remarks The `value` boolean is true if the given `T` has a function named @c
  /// MaybeReplacementCodeUnits() on it that can be called from a `const` -qualified `T` which returns
  /// a `std::optional` containing a contiguous view of code units.
  template <typename T, typename... TArgs>
  concept HasMaybeReplacementCodeUnits =
    requires { std::declval<const T &>().MaybeReplacementCodeUnits(std::declval<TArgs>()...); };

  /// @brief Checks whether the given encoding type returns a replacement range of code units.
  /// @tparam T The type to check.
  /// @remarks The `value` boolean is true if the given `T` has a function named
  /// `ReplacementCodeUnits()` on it that can be called from a `const`-qualified `T` which returns a
  /// contiguous view of code units.
  template <typename T, typename... TArgs>
  concept HasReplacementCodeUnits =
    requires { std::declval<const T &>().ReplacementCodeUnits(std::declval<TArgs>()...); };
}
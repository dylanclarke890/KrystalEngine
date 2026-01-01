#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"

namespace Krys
{
  /// @brief Checks whether the given encoding type returns a maybe-replacement range of code points.
  /// @tparam T The type to check.
  /// @remarks The `value` boolean is true if the given `T` has a function named @c
  /// MaybeReplacementCodePoints() on it that can be called from a `const` -qualified `T` object
  /// which returns a `std::optional` containing a contiguous view of code points.
  template <typename T, typename... TArgs>
  concept HasMaybeReplacementCodePoints =
    requires { std::declval<const T &>().MaybeReplacementCodePoints(std::declval<TArgs>()...); };

  /// @brief Checks whether the given encoding type returns a maybe-replacement range of code points.
  /// @tparam T The type to check.
  /// @remarks The `value` boolean is true if the given `T` has a function named @c
  /// ReplacementCodePoints() on it that can be called from a `const` -qualified `T` object which
  /// returns a contiguous view of code points.
  template <typename T, typename... TArgs>
  concept HasReplacementCodePoints =
    requires { std::declval<const T &>().ReplacementCodePoints(std::declval<TArgs>()...); };
}
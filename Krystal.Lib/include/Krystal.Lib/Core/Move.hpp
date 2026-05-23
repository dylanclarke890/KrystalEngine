#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"

namespace Krys
{
  /// @brief Same as std::move, but with additional static assertions to catch misuse at compile time.
  template <typename T>
  KRYS_NODISCARD constexpr inline remove_ref_t<T> &&Move(T &&value)
  {
    static_assert(LValueRef<T>, "T is not an lvalue reference; Move() is unnecessary.");
    static_assert(!IsConst<remove_ref_t<T>>, "T is const qualified.");

    return std::move(std::forward<T>(value));
  }
}
#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include <xutility>

namespace krys
{
  template <typename T>
  concept NoThrowMove = requires(T &&value) { noexcept(std::move(std::forward<T>(value))); };

  /// @brief Same as std::move, but with additional static assertions to catch misuse at compile time.
  template <typename T>
  KRYS_NODISCARD constexpr remove_ref_t<T> &&move(T &&value) noexcept(NoThrowMove<T>)
  {
    static_assert(LValueRef<T>, "T is not an lvalue reference; move() is unnecessary.");
    static_assert(!IsConst<remove_ref_t<T>>, "T is const qualified.");
    return std::move(std::forward<T>(value));
  }
}
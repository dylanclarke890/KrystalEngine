#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Matrix.hpp"
#include "Krystal.Core/Maths/Traits.hpp"
#include "Krystal.Core/Maths/Vector.hpp"

namespace krys
{
  /// @brief Whether each component of @p a is less than the matching component of @p b.
  template <VectorOrMatrix TValue>
  KRYS_NODISCARD constexpr auto LessThan(const TValue &a, const TValue &b) noexcept
  {
    return Zip(a, b, [](auto x, auto y) -> bool { return x < y; });
  }

  /// @brief Whether every component of @p a is less than @p b.
  template <VectorOrMatrix TValue>
  KRYS_NODISCARD constexpr bool LessThan(const TValue &a, typename Traits<TValue>::ComponentType b) noexcept
  {
    return AllOf(a, [&b](auto v) { return v < b; });
  }

  /// @brief Whether each component of @p a is at most the matching component of @p b.
  template <VectorOrMatrix TValue>
  KRYS_NODISCARD constexpr auto LessThanOrEqual(const TValue &a, const TValue &b) noexcept
  {
    return Zip(a, b, [](auto x, auto y) -> bool { return x <= y; });
  }

  /// @brief Whether every component of @p a is at most @p b.
  template <VectorOrMatrix TValue>
  KRYS_NODISCARD constexpr bool LessThanOrEqual(const TValue &a,
                                                typename Traits<TValue>::ComponentType b) noexcept
  {
    return AllOf(a, [&b](auto v) { return v <= b; });
  }

  /// @brief Whether each component of @p a is greater than the matching component of @p b.
  template <VectorOrMatrix TValue>
  KRYS_NODISCARD constexpr auto GreaterThan(const TValue &a, const TValue &b) noexcept
  {
    return Zip(a, b, [](auto x, auto y) -> bool { return x > y; });
  }

  /// @brief Whether every component of @p a is greater than @p b.
  template <VectorOrMatrix TValue>
  KRYS_NODISCARD constexpr bool GreaterThan(const TValue &a,
                                            typename Traits<TValue>::ComponentType b) noexcept
  {
    return AllOf(a, [&b](auto v) { return v > b; });
  }

  /// @brief Whether each component of @p a is at least the matching component of @p b.
  template <VectorOrMatrix TValue>
  KRYS_NODISCARD constexpr auto GreaterThanOrEqual(const TValue &a, const TValue &b) noexcept
  {
    return Zip(a, b, [](auto x, auto y) -> bool { return x >= y; });
  }

  /// @brief Whether every component of @p a is at least @p b.
  template <VectorOrMatrix TValue>
  KRYS_NODISCARD constexpr bool GreaterThanOrEqual(const TValue &a,
                                                   typename Traits<TValue>::ComponentType b) noexcept
  {
    return AllOf(a, [&b](auto v) { return v >= b; });
  }

  /// @brief Whether each component of @p a is equal to the matching component of @p b.
  template <VectorOrMatrix TValue>
  KRYS_NODISCARD constexpr auto Equal(const TValue &a, const TValue &b) noexcept
  {
    return Zip(a, b, [](auto x, auto y) -> bool { return x == y; });
  }

  /// @brief Whether every component of @p a is equal to @p b.
  template <VectorOrMatrix TValue>
  KRYS_NODISCARD constexpr bool Equal(const TValue &a, typename Traits<TValue>::ComponentType b) noexcept
  {
    return AllOf(a, [&b](auto v) { return v == b; });
  }

  /// @brief Whether each component of @p a is not equal to the matching component of @p b.
  template <VectorOrMatrix TValue>
  KRYS_NODISCARD constexpr auto NotEqual(const TValue &a, const TValue &b) noexcept
  {
    return Zip(a, b, [](auto x, auto y) -> bool { return x != y; });
  }

  /// @brief Whether every component of @p a is not equal to @p b.
  template <VectorOrMatrix TValue>
  KRYS_NODISCARD constexpr bool NotEqual(const TValue &a, typename Traits<TValue>::ComponentType b) noexcept
  {
    return AllOf(a, [&b](auto v) { return v != b; });
  }
}

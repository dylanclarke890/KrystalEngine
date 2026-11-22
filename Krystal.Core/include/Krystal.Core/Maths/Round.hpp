#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Maths/Matrix.hpp"
#include "Krystal.Core/Maths/Traits.hpp"
#include "Krystal.Core/Maths/Vector.hpp"

namespace krys
{
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T Ceil(T x) noexcept
  {
    if (std::is_constant_evaluated())
    {
      if (static_cast<long long>(x) == x)
      {
        return x;
      }

      return static_cast<long long>(x) + ((x < T(0)) ? T(0) : T(1));
    }

    return std::ceil(x);
  }

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T Floor(T x) noexcept
  {
    if (std::is_constant_evaluated())
    {
      if (static_cast<long long>(x) == x)
      {
        return x;
      }

      return static_cast<T>(static_cast<long long>(x) - ((x < T(0)) ? 1ll : 0ll));
    }
    else
    {
      return std::floor(x);
    }
  }

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T Round(T x) noexcept
  {
    if (std::is_constant_evaluated())
    {
      return T(static_cast<long long>(x + (x > T(0) ? T(0.5) : T(-0.5))));
    }

    return std::round(x);
  }

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T Trunc(T x) noexcept
  {
    if (std::is_constant_evaluated())
    {
      return T(static_cast<long long>(x));
    }

    return std::trunc(x);
  }

  /// @brief Each component of @p x rounded up.
  template <VectorOrMatrix TValue>
  requires FloatingPoint<typename Traits<TValue>::ComponentType>
  KRYS_NODISCARD constexpr auto Ceil(const TValue &x) noexcept
  {
    return MapEach(x, [](auto v) { return Ceil(v); });
  }

  /// @brief Each component of @p x rounded down.
  template <VectorOrMatrix TValue>
  requires FloatingPoint<typename Traits<TValue>::ComponentType>
  KRYS_NODISCARD constexpr auto Floor(const TValue &x) noexcept
  {
    return MapEach(x, [](auto v) { return Floor(v); });
  }

  /// @brief Each component of @p x rounded to the nearest integer, halves away from zero.
  template <VectorOrMatrix TValue>
  requires FloatingPoint<typename Traits<TValue>::ComponentType>
  KRYS_NODISCARD constexpr auto Round(const TValue &x) noexcept
  {
    return MapEach(x, [](auto v) { return Round(v); });
  }

  /// @brief Each component of @p x rounded towards zero.
  template <VectorOrMatrix TValue>
  requires FloatingPoint<typename Traits<TValue>::ComponentType>
  KRYS_NODISCARD constexpr auto Trunc(const TValue &x) noexcept
  {
    return MapEach(x, [](auto v) { return Trunc(v); });
  }
}

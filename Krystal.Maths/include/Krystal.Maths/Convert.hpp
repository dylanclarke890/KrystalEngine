#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"

namespace Krys::Maths
{
  /// @brief Converts radians to degrees.
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T Degrees(T radians) noexcept
  {
    return T(static_cast<double>(radians) * 57.295779513082320876798154814105);
  }

  KRYS_NODISCARD constexpr long double operator""_degrees(long double value)
  {
    return Degrees(value);
  }

  /// @brief Converts degrees to radians.
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T Radians(T degrees) noexcept
  {
    return degrees * static_cast<T>(0.01745329251994329576923690768489);
  }

  KRYS_NODISCARD constexpr long double operator""_radians(long double value)
  {
    return Radians(value);
  }
}
#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Maths
{
  template <typename T>
  KRYS_NODISCARD constexpr T Gauss(T x, T expected, T stdDeviation) noexcept
  {
    return std::exp(-((x - expected) * (x - expected)) / (T(2) * stdDeviation * stdDeviation))
           / (stdDeviation * std::sqrt(T(6.28318530717958647692528676655900576)));
  }

  template <typename T>
  KRYS_NODISCARD constexpr T Gauss(const Vector2<T> &coord, const Vector2<T> &expected,
                               const Vector2<T> &stdDeviation) noexcept
  {
    const Vector2<T> squared =
      ((coord - expected) * (coord - expected)) / (T(2) * stdDeviation * stdDeviation);
    return std::exp(-(squared.x + squared.y));
  }
}
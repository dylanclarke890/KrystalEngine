#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include <cmath>
#include <limits>

namespace krys::boo
{
  /// @brief Divides `u` by `v`, protecting against overflow and underflow.
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T SafeFPDivision(T u, T v) noexcept
  {
    // Protect against overflow / underflow.
    if (v < 1 && u > v * std::numeric_limits<T>::max())
    {
      return std::numeric_limits<T>::max();
    }

    if (v > 1 && u < v * std::numeric_limits<T>::min())
    {
      return 0;
    }

    return u / v;
  }

  /// @brief Floating point numbers comparison: `u` is "essentially equal" [1][2] to `v` if
  /// | u - v | / |u| <= e and | u - v | / |v| <= e.
  ///
  /// [1] Knuth, D. E. "Accuracy of Floating Point Arithmetic." The Art of Computer Programming. 3rd ed.
  /// Vol. 2.
  ///     Boston: Addison-Wesley, 1998. 229-45.
  /// [2]
  /// http://www.boost.org/doc/libs/1_34_0/libs/test/doc/components/test_tools/floating_point_comparison.html
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr bool AreEssentiallyEqual(T u, T v,
                                                    T epsilon = std::numeric_limits<T>::epsilon()) noexcept
  {
    if (u == v)
    {
      return true;
    }

    const T delta = std::abs(u - v);
    return SafeFPDivision(delta, std::abs(u)) <= epsilon && SafeFPDivision(delta, std::abs(v)) <= epsilon;
  }
}

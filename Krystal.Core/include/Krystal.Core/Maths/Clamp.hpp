#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Maths/Maths.hpp"
#include "Krystal.Core/Maths/Matrix.hpp"
#include "Krystal.Core/Maths/Vector.hpp"
#include <algorithm>
#include <limits>

namespace krys
{
#pragma region Min

  template <Number T>
  KRYS_NODISCARD constexpr T Min(T a, T b) noexcept
  {
    return a < b ? a : b;
  }

  template <Number T>
  KRYS_NODISCARD constexpr T Min(T a, T b, T c) noexcept
  {
    return Min(Min(a, b), c);
  }

  template <Number T>
  KRYS_NODISCARD constexpr T Min(T a, T b, T c, T d) noexcept
  {
    return Min(Min(a, b), Min(c, d));
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr T Min(const Vector<T, N> &v) noexcept
  {
    T min = std::numeric_limits<T>::max();
    ForEach(v, [&min](auto val) { min = Min(min, val); });
    return min;
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr T Min(const Matrix<T, C, R> &v) noexcept
  {
    T min = std::numeric_limits<T>::max();
    ForEach(v, [&min](auto val) { min = Min(min, val); });
    return min;
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr auto Min(const Vector<T, N> &a, const Vector<T, N> &b) noexcept
  {
    return Zip(a, b, [](T x, T y) -> T { return Min(x, y); });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto Min(const Matrix<T, C, R> &a, const Matrix<T, C, R> &b) noexcept
  {
    return Zip(a, b, [](T x, T y) -> T { return Min(x, y); });
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr auto Min(const Vector<T, N> &a, const Vector<T, N> &b,
                                    const Vector<T, N> &c) noexcept
  {
    return Zip(a, b, c, [](T x, T y, T z) -> T { return Min(x, y, z); });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto Min(const Matrix<T, C, R> &a, const Matrix<T, C, R> &b,
                                    const Matrix<T, C, R> &c) noexcept
  {
    return Zip(a, b, c, [](T x, T y, T z) -> T { return Min(x, y, z); });
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr auto Min(const Vector<T, N> &a, T b) noexcept
  {
    return MapEach(a, [&b](T v) -> T { return Min(v, b); });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto Min(const Matrix<T, C, R> &a, T b) noexcept
  {
    return MapEach(a, [&b](T v) -> T { return Min(v, b); });
  }

#pragma endregion

#pragma region Max

  template <Number T>
  KRYS_NODISCARD constexpr T Max(T a, T b) noexcept
  {
    return a > b ? a : b;
  }

  template <Number T>
  KRYS_NODISCARD constexpr T Max(T a, T b, T c) noexcept
  {
    return Max(Max(a, b), c);
  }

  template <Number T>
  KRYS_NODISCARD constexpr T Max(T a, T b, T c, T d) noexcept
  {
    return Max(Max(a, b), Max(c, d));
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr T Max(const Vector<T, N> &v) noexcept
  {
    T max = std::numeric_limits<T>::lowest();
    ForEach(v, [&max](auto val) { max = Max(max, val); });
    return max;
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr T Max(const Matrix<T, C, R> &v) noexcept
  {
    T max = std::numeric_limits<T>::lowest();
    ForEach(v, [&max](auto val) { max = Max(max, val); });
    return max;
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr auto Max(const Vector<T, N> &a, const Vector<T, N> &b) noexcept
  {
    return Zip(a, b, [](T x, T y) -> T { return Max(x, y); });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto Max(const Matrix<T, C, R> &a, const Matrix<T, C, R> &b) noexcept
  {
    return Zip(a, b, [](T x, T y) -> T { return Max(x, y); });
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr auto Max(const Vector<T, N> &a, const Vector<T, N> &b,
                                    const Vector<T, N> &c) noexcept
  {
    return Zip(a, b, c, [](T x, T y, T z) -> T { return Max(x, y, z); });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto Max(const Matrix<T, C, R> &a, const Matrix<T, C, R> &b,
                                    const Matrix<T, C, R> &c) noexcept
  {
    return Zip(a, b, c, [](T x, T y, T z) -> T { return Max(x, y, z); });
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr auto Max(const Vector<T, N> &a, T b) noexcept
  {
    return MapEach(a, [&b](T v) -> T { return Max(v, b); });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto Max(const Matrix<T, C, R> &a, T b) noexcept
  {
    return MapEach(a, [&b](T v) -> T { return Max(v, b); });
  }

#pragma endregion

#pragma region FMin

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T FMin(T a, T b) noexcept
  {
    if (IsNaN(a))
    {
      return b;
    }

    return Min(a, b);
  }

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T FMin(T a, T b, T c) noexcept
  {
    if (IsNaN(a))
    {
      return FMin(b, c);
    }
    if (IsNaN(b))
    {
      return FMin(a, c);
    }
    else if (IsNaN(c))
    {
      return Min(a, b);
    }
    else
    {
      return Min(a, b, c);
    }
  }

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T FMin(T a, T b, T c, T d) noexcept
  {
    if (IsNaN(a))
    {
      return FMin(b, c, d);
    }
    if (IsNaN(b))
    {
      return Min(a, FMin(c, d));
    }
    else if (IsNaN(c))
    {
      return FMin(Min(a, b), d);
    }
    else if (IsNaN(d))
    {
      return Min(a, b, c);
    }
    else
    {
      return Min(a, b, c, d);
    }
  }

  template <FloatingPoint T, int N>
  KRYS_NODISCARD constexpr auto FMin(const Vector<T, N> &v) noexcept
  {
    T min = std::numeric_limits<T>::max();
    ForEach(v, [&min](auto val) { min = FMin(min, val); });
    return min;
  }

  template <FloatingPoint T, int R, int C>
  KRYS_NODISCARD constexpr auto FMin(const Matrix<T, C, R> &m) noexcept
  {
    T min = std::numeric_limits<T>::max();
    ForEach(m, [&min](auto val) { min = FMin(min, val); });
    return min;
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr auto FMin(const Vector<T, N> &a, const Vector<T, N> &b) noexcept
  {
    return Zip(a, b, [](T x, T y) -> T { return FMin(x, y); });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto FMin(const Matrix<T, C, R> &a, const Matrix<T, C, R> &b) noexcept
  {
    return Zip(a, b, [](T x, T y) -> T { return FMin(x, y); });
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr auto FMin(const Vector<T, N> &a, const Vector<T, N> &b,
                                     const Vector<T, N> &c) noexcept
  {
    return Zip(a, b, c, [](T x, T y, T z) -> T { return FMin(x, y, z); });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto FMin(const Matrix<T, C, R> &a, const Matrix<T, C, R> &b,
                                     const Matrix<T, C, R> &c) noexcept
  {
    return Zip(a, b, c, [](T x, T y, T z) -> T { return FMin(x, y, z); });
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr auto FMin(const Vector<T, N> &a, T b) noexcept
  {
    return MapEach(a, [&b](T v) -> T { return FMin(v, b); });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto FMin(const Matrix<T, C, R> &a, T b) noexcept
  {
    return MapEach(a, [&b](T v) -> T { return FMin(v, b); });
  }

#pragma endregion

#pragma region FMax

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T FMax(T a, T b) noexcept
  {
    if (IsNaN(a))
    {
      return b;
    }

    return Max(a, b);
  }

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T FMax(T a, T b, T c) noexcept
  {
    if (IsNaN(a))
    {
      return FMax(b, c);
    }
    if (IsNaN(b))
    {
      return FMax(a, c);
    }
    else if (IsNaN(c))
    {
      return Max(a, b);
    }
    else
    {
      return Max(a, b, c);
    }
  }

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T FMax(T a, T b, T c, T d) noexcept
  {
    if (IsNaN(a))
    {
      return FMax(b, c, d);
    }
    if (IsNaN(b))
    {
      return Max(a, FMax(c, d));
    }
    else if (IsNaN(c))
    {
      return FMax(Max(a, b), d);
    }
    else if (IsNaN(d))
    {
      return Max(a, b, c);
    }
    else
    {
      return Max(a, b, c, d);
    }
  }

  template <FloatingPoint T, int N>
  KRYS_NODISCARD constexpr auto FMax(const Vector<T, N> &v) noexcept
  {
    T max = std::numeric_limits<T>::lowest();
    ForEach(v, [&max](auto val) { max = FMax(max, val); });
    return max;
  }

  template <FloatingPoint T, int R, int C>
  KRYS_NODISCARD constexpr auto FMax(const Matrix<T, C, R> &m) noexcept
  {
    T max = std::numeric_limits<T>::lowest();
    ForEach(m, [&max](auto val) { max = FMax(max, val); });
    return max;
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr auto FMax(const Vector<T, N> &a, const Vector<T, N> &b) noexcept
  {
    return Zip(a, b, [](T x, T y) -> T { return FMax(x, y); });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto FMax(const Matrix<T, C, R> &a, const Matrix<T, C, R> &b) noexcept
  {
    return Zip(a, b, [](T x, T y) -> T { return FMax(x, y); });
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr auto FMax(const Vector<T, N> &a, const Vector<T, N> &b,
                                     const Vector<T, N> &c) noexcept
  {
    return Zip(a, b, c, [](T x, T y, T z) -> T { return FMax(x, y, z); });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto FMax(const Matrix<T, C, R> &a, const Matrix<T, C, R> &b,
                                     const Matrix<T, C, R> &c) noexcept
  {
    return Zip(a, b, c, [](T x, T y, T z) -> T { return FMax(x, y, z); });
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr auto FMax(const Vector<T, N> &a, T b) noexcept
  {
    return MapEach(a, [&b](T v) -> T { return FMax(v, b); });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto FMax(const Matrix<T, C, R> &a, T b) noexcept
  {
    return MapEach(a, [&b](T v) -> T { return FMax(v, b); });
  }

#pragma endregion

#pragma region Clamp

  template <Number T>
  KRYS_NODISCARD constexpr T Clamp(T value, T min, T max) noexcept
  {
    return std::clamp(value, min, max);
  }

  /// @brief Clamps each component of the vector between the corresponding components of the `min` and `max`
  /// vectors.
  template <Number T, int N>
  KRYS_NODISCARD constexpr auto Clamp(const Vector<T, N> &value, const Vector<T, N> &min,
                                      const Vector<T, N> &max) noexcept
  {
    return Zip(value, min, max, [](auto v, auto lo, auto hi) -> T { return Clamp(v, lo, hi); });
  }

  /// @brief Clamps each component of the matrix between the corresponding components of the `min` and `max`
  /// matrices.
  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto Clamp(const Matrix<T, C, R> &value, const Matrix<T, C, R> &min,
                                      const Matrix<T, C, R> &max) noexcept
  {
    return Zip(value, min, max, [](auto v, auto lo, auto hi) -> T { return Clamp(v, lo, hi); });
  }

  /// @brief Clamps each component of the vector between the scalar values `min` and `max`.
  template <Number T, int N>
  KRYS_NODISCARD constexpr auto Clamp(const Vector<T, N> &value, T min, T max) noexcept
  {
    return MapEach(value, [&min, &max](auto v) -> T { return Clamp(v, min, max); });
  }

  /// @brief Clamps each component of the matrix between the scalar values `min` and `max`.
  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto Clamp(const Matrix<T, C, R> &value, T min, T max) noexcept
  {
    return MapEach(value, [&min, &max](auto v) -> T { return Clamp(v, min, max); });
  }

#pragma endregion
}

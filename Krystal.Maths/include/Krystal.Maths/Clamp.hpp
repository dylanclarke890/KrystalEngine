#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Maths/Maths.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <algorithm>
#include <limits>

namespace Krys::Maths
{
#pragma region Min

  template <Arithmetic T>
  NO_DISCARD constexpr T Min(T a, T b) noexcept
  {
    return a < b ? a : b;
  }

  template <Arithmetic T>
  NO_DISCARD constexpr T Min(T a, T b, T c) noexcept
  {
    return Min(Min(a, b), c);
  }

  template <Arithmetic T>
  NO_DISCARD constexpr T Min(T a, T b, T c, T d) noexcept
  {
    return Min(Min(a, b), Min(c, d));
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr T Min(const VECTOR_TYPE &v) noexcept
  {
    T min = std::numeric_limits<T>::max();
    ForEach(v, [&min](auto val) { min = Min(min, val); });
    return min;
  }

  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr T Min(const MATRIX_TYPE &v) noexcept
  {
    T min = std::numeric_limits<T>::max();
    ForEach(v, [&min](auto val) { min = Min(min, val); });
    return min;
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Min(const VECTOR_TYPE &a, const VECTOR_TYPE &b) noexcept
  {
    return Zip(a, b, [](T x, T y) -> T { return Min(x, y); });
  }

  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Min(const MATRIX_TYPE &a, const MATRIX_TYPE &b) noexcept
  {
    return Zip(a, b, [](T x, T y) -> T { return Min(x, y); });
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Min(const VECTOR_TYPE &a, const VECTOR_TYPE &b, const VECTOR_TYPE &c) noexcept
  {
    return Zip(a, b, c, [](T x, T y, T z) -> T { return Min(x, y, z); });
  }

  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Min(const MATRIX_TYPE &a, const MATRIX_TYPE &b, const MATRIX_TYPE &c) noexcept
  {
    return Zip(a, b, c, [](T x, T y, T z) -> T { return Min(x, y, z); });
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Min(const VECTOR_TYPE &a, T b) noexcept
  {
    return MapEach(a, [&b](T v) -> T { return Min(v, b); });
  }

  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Min(const MATRIX_TYPE &a, T b) noexcept
  {
    return MapEach(a, [&b](T v) -> T { return Min(v, b); });
  }

#pragma endregion

#pragma region Max

  template <Arithmetic T>
  NO_DISCARD constexpr T Max(T a, T b) noexcept
  {
    return a > b ? a : b;
  }

  template <Arithmetic T>
  NO_DISCARD constexpr T Max(T a, T b, T c) noexcept
  {
    return Max(Max(a, b), c);
  }

  template <Arithmetic T>
  NO_DISCARD constexpr T Max(T a, T b, T c, T d) noexcept
  {
    return Max(Max(a, b), Max(c, d));
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr T Max(const VECTOR_TYPE &v) noexcept
  {
    T max = std::numeric_limits<T>::lowest();
    ForEach(v, [&max](auto val) { max = Max(max, val); });
    return max;
  }

  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr T Max(const MATRIX_TYPE &v) noexcept
  {
    T max = std::numeric_limits<T>::lowest();
    ForEach(v, [&max](auto val) { max = Max(max, val); });
    return max;
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Max(const VECTOR_TYPE &a, const VECTOR_TYPE &b) noexcept
  {
    return Zip(a, b, [](T x, T y) -> T { return Max(x, y); });
  }

  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Max(const MATRIX_TYPE &a, const MATRIX_TYPE &b) noexcept
  {
    return Zip(a, b, [](T x, T y) -> T { return Max(x, y); });
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Max(const VECTOR_TYPE &a, const VECTOR_TYPE &b, const VECTOR_TYPE &c) noexcept
  {
    return Zip(a, b, c, [](T x, T y, T z) -> T { return Max(x, y, z); });
  }

  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Max(const MATRIX_TYPE &a, const MATRIX_TYPE &b, const MATRIX_TYPE &c) noexcept
  {
    return Zip(a, b, c, [](T x, T y, T z) -> T { return Max(x, y, z); });
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Max(const VECTOR_TYPE &a, T b) noexcept
  {
    return MapEach(a, [&b](T v) -> T { return Max(v, b); });
  }

  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Max(const MATRIX_TYPE &a, T b) noexcept
  {
    return MapEach(a, [&b](T v) -> T { return Max(v, b); });
  }

#pragma endregion

#pragma region FMin

  template <FloatingPoint T>
  NO_DISCARD constexpr T FMin(T a, T b) noexcept
  {
    if (IsNaN(a))
      return b;
    else
      return Min(a, b);
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T FMin(T a, T b, T c) noexcept
  {
    if (IsNaN(a))
      return FMin(b, c);
    else if (IsNaN(b))
      return FMin(a, c);
    else if (IsNaN(c))
      return Min(a, b);
    else
      return Min(a, b, c);
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T FMin(T a, T b, T c, T d) noexcept
  {
    if (IsNaN(a))
      return FMin(b, c, d);
    else if (IsNaN(b))
      return Min(a, FMin(c, d));
    else if (IsNaN(c))
      return FMin(Min(a, b), d);
    else if (IsNaN(d))
      return Min(a, b, c);
    else
      return Min(a, b, c, d);
  }

  template <FloatingPoint T, int N>
  NO_DISCARD constexpr auto FMin(const VECTOR_TYPE &v) noexcept
  {
    T min = std::numeric_limits<T>::max();
    ForEach(v, [&min](auto val) { min = FMin(min, val); });
    return min;
  }

  template <FloatingPoint T, int R, int C>
  NO_DISCARD constexpr auto FMin(const MATRIX_TYPE &m) noexcept
  {
    T min = std::numeric_limits<T>::max();
    ForEach(m, [&min](auto val) { min = FMin(min, val); });
    return min;
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto FMin(const VECTOR_TYPE &a, const VECTOR_TYPE &b) noexcept
  {
    return Zip(a, b, [](T x, T y) -> T { return FMin(x, y); });
  }

  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto FMin(const MATRIX_TYPE &a, const MATRIX_TYPE &b) noexcept
  {
    return Zip(a, b, [](T x, T y) -> T { return FMin(x, y); });
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto FMin(const VECTOR_TYPE &a, const VECTOR_TYPE &b, const VECTOR_TYPE &c) noexcept
  {
    return Zip(a, b, c, [](T x, T y, T z) -> T { return FMin(x, y, z); });
  }

  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto FMin(const MATRIX_TYPE &a, const MATRIX_TYPE &b, const MATRIX_TYPE &c) noexcept
  {
    return Zip(a, b, c, [](T x, T y, T z) -> T { return FMin(x, y, z); });
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto FMin(const VECTOR_TYPE &a, T b) noexcept
  {
    return MapEach(a, [&b](T v) -> T { return FMin(v, b); });
  }

  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto FMin(const MATRIX_TYPE &a, T b) noexcept
  {
    return MapEach(a, [&b](T v) -> T { return FMin(v, b); });
  }

#pragma endregion

#pragma region FMax

  template <FloatingPoint T>
  NO_DISCARD constexpr T FMax(T a, T b) noexcept
  {
    if (IsNaN(a))
      return b;
    else
      return Max(a, b);
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T FMax(T a, T b, T c) noexcept
  {
    if (IsNaN(a))
      return FMax(b, c);
    else if (IsNaN(b))
      return FMax(a, c);
    else if (IsNaN(c))
      return Max(a, b);
    else
      return Max(a, b, c);
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T FMax(T a, T b, T c, T d) noexcept
  {
    if (IsNaN(a))
      return FMax(b, c, d);
    else if (IsNaN(b))
      return Max(a, FMax(c, d));
    else if (IsNaN(c))
      return FMax(Max(a, b), d);
    else if (IsNaN(d))
      return Max(a, b, c);
    else
      return Max(a, b, c, d);
  }

  template <FloatingPoint T, int N>
  NO_DISCARD constexpr auto FMax(const VECTOR_TYPE &v) noexcept
  {
    T max = std::numeric_limits<T>::lowest();
    ForEach(v, [&max](auto val) { max = FMax(max, val); });
    return max;
  }

  template <FloatingPoint T, int R, int C>
  NO_DISCARD constexpr auto FMax(const MATRIX_TYPE &m) noexcept
  {
    T max = std::numeric_limits<T>::lowest();
    ForEach(m, [&max](auto val) { max = FMax(max, val); });
    return max;
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto FMax(const VECTOR_TYPE &a, const VECTOR_TYPE &b) noexcept
  {
    return Zip(a, b, [](T x, T y) -> T { return FMax(x, y); });
  }

  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto FMax(const MATRIX_TYPE &a, const MATRIX_TYPE &b) noexcept
  {
    return Zip(a, b, [](T x, T y) -> T { return FMax(x, y); });
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto FMax(const VECTOR_TYPE &a, const VECTOR_TYPE &b, const VECTOR_TYPE &c) noexcept
  {
    return Zip(a, b, c, [](T x, T y, T z) -> T { return FMax(x, y, z); });
  }

  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto FMax(const MATRIX_TYPE &a, const MATRIX_TYPE &b, const MATRIX_TYPE &c) noexcept
  {
    return Zip(a, b, c, [](T x, T y, T z) -> T { return FMax(x, y, z); });
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto FMax(const VECTOR_TYPE &a, T b) noexcept
  {
    return MapEach(a, [&b](T v) -> T { return FMax(v, b); });
  }

  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto FMax(const MATRIX_TYPE &a, T b) noexcept
  {
    return MapEach(a, [&b](T v) -> T { return FMax(v, b); });
  }

#pragma endregion

#pragma region Clamp

  template <Arithmetic T>
  NO_DISCARD constexpr T Clamp(T value, T min, T max) noexcept
  {
    return std::clamp(value, min, max);
  }

  /// @brief Clamps each component of the vector between the corresponding components of the `min` and `max`
  /// vectors.
  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Clamp(const VECTOR_TYPE &value, const VECTOR_TYPE &min,
                                  const VECTOR_TYPE &max) noexcept
  {
    return Zip(value, min, max, [](auto v, auto lo, auto hi) -> T { return Clamp(v, lo, hi); });
  }

  /// @brief Clamps each component of the matrix between the corresponding components of the `min` and `max`
  /// matrices.
  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Clamp(const MATRIX_TYPE &value, const MATRIX_TYPE &min,
                                  const MATRIX_TYPE &max) noexcept
  {
    return Zip(value, min, max, [](auto v, auto lo, auto hi) -> T { return Clamp(v, lo, hi); });
  }

  /// @brief Clamps each component of the vector between the scalar values `min` and `max`.
  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Clamp(const VECTOR_TYPE &value, T min, T max) noexcept
  {
    return MapEach(value, [&min, &max](auto v) -> T { return Clamp(v, min, max); });
  }

  /// @brief Clamps each component of the matrix between the scalar values `min` and `max`.
  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Clamp(const MATRIX_TYPE &value, T min, T max) noexcept
  {
    return MapEach(value, [&min, &max](auto v) -> T { return Clamp(v, min, max); });
  }

#pragma endregion
}
#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Maths/Clamp.hpp"
#include "Krystal.Maths/Maths.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Quaternion.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Maths
{
  template <Number T, FloatingPoint U>
  KRYS_NODISCARD constexpr T Lerp(T x, T y, U t) noexcept
  {
    t = Clamp(t, Zero<U>(), One<U>());
    auto result = U(x) * (One<U>() - t) + (U(y) * t);
    return static_cast<T>(result);
  }

  template <Number T, FloatingPoint U>
  KRYS_NODISCARD constexpr T Smoothstep(T x, T y, U t) noexcept
  {
    t = Clamp((t - static_cast<U>(x)) / static_cast<U>(y - x), Zero<U>(), One<U>());
    auto result = t * t * (U(3) - U(2) * t);
    return static_cast<T>(result);
  }

  template <Number T, FloatingPoint U>
  KRYS_NODISCARD constexpr T Step(T x, T y, U t) noexcept
  {
    if (t < Half<U>())
      return x;
    return y;
  }

  template <Number T, Number U>
  KRYS_NODISCARD constexpr T Step(T edge, U x) noexcept
  {
    if (x < static_cast<U>(edge))
      return Zero<T>();
    return One<T>();
  }

  template <VECTOR_TEMPLATE_PARAMS>
  KRYS_NODISCARD constexpr auto Lerp(const VECTOR_TYPE &x, const VECTOR_TYPE &y, const VECTOR_TYPE &t) noexcept
  {
    auto weight = Clamp(t, Zero<T>(), One<T>());
    return (x * (One<T>() - weight)) + (y * weight);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  KRYS_NODISCARD constexpr auto Lerp(const VECTOR_TYPE &x, const VECTOR_TYPE &y, T t) noexcept
  {
    t = Clamp(t, Zero<T>(), One<T>());
    return (x * (One<T>() - t)) + (y * t);
  }

  template <MATRIX_TEMPLATE_PARAMS>
  KRYS_NODISCARD constexpr auto Lerp(const MATRIX_TYPE &x, const MATRIX_TYPE &y, const MATRIX_TYPE &t) noexcept
  {
    auto weight = Clamp(t, Zero<T>(), One<T>());
    return (x * (One<T>() - weight)) + (y * weight);
  }

  template <MATRIX_TEMPLATE_PARAMS>
  KRYS_NODISCARD constexpr auto Lerp(const MATRIX_TYPE &x, const MATRIX_TYPE &y, T t) noexcept
  {
    t = Clamp(t, Zero<T>(), One<T>());
    return (x * (One<T>() - t)) + (y * t);
  }

  /// @brief Spherically interpolates between two quaternions.
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr auto Slerp(const Quaternion<T> &a, const Quaternion<T> &b, T t) noexcept
  {
    auto q1 = a;
    auto q2 = b;

    T dot = q1.Dot(q2);
    if (dot < 0.0f)
    {
      q2 = -q2;
      dot = -dot;
    }

    constexpr T threshold = 0.9995f;
    if (dot > threshold)
    {
      auto result = q1 + (q2 - q1) * t;
      result.Normalize();
      return result;
    }

    dot = Clamp(dot, -1.0f, 1.0f);
    T theta = std::acos(dot);
    T sinTheta = std::sin(theta);
    T sinTTheta = std::sin(t * theta);

    T s0 = std::cos(t * theta) - dot * sinTTheta / sinTheta;
    T s1 = sinTTheta / sinTheta;

    return q1 * s0 + q2 * s1;
  }
}

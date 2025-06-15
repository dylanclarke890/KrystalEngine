#pragma once

#include "Krystal.Core/Core.hpp"

#include <cassert>

namespace Krys::Maths
{
  template <FloatingPoint T>
  NO_DISCARD constexpr T LinearInterpolation(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    return value;
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T QuadraticEaseIn(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    return value * value;
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T QuadraticEaseOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    return -(value * (value - T(2)));
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T QuadraticEaseInOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");

    if (value < T(0.5))
      return T(2) * value * value;
    else
      return (-T(2) * value * value) + (4 * value) - One<T>();
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T CubicEaseIn(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    return value * value * value;
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T CubicEaseOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    const T f = value - One<T>();
    return f * f * f + One<T>();
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T CubicEaseInOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");

    if (value < T(0.5))
      return T(4) * value * value * value;
    else
    {
      const T f = ((T(2) * value) - T(2));
      return T(0.5) * f * f * f + One<T>();
    }
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T QuarticEaseIn(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    return value * value * value * value;
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T QuarticEaseOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    const T f = (value - One<T>());
    return f * f * f * (One<T>() - value) + One<T>();
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T QuarticEaseInOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");

    if (value < T(0.5))
      return T(8) * value * value * value * value;
    else
    {
      const T f = (value - One<T>());
      return -T(8) * f * f * f * f + One<T>();
    }
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T QuinticEaseIn(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    return value * value * value * value * value;
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T QuinticEaseOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    const T f = (value - One<T>());
    return f * f * f * f * f + One<T>();
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T QuinticEaseInOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");

    if (value < T(0.5))
      return T(16) * value * value * value * value * value;
    else
    {
      const T f = ((T(2) * value) - T(2));
      return T(0.5) * f * f * f * f * f + One<T>();
    }
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T SineEaseIn(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    return Sin((value - One<T>()) * HalfPi<T>()) + One<T>();
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T SineEaseOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    return Sin(value * HalfPi<T>());
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T SineEaseInOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    return T(0.5) * (One<T>() - Cos(value * Pi<T>()));
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T CircularEaseIn(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    return One<T>() - Sqrt(One<T>() - (value * value));
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T CircularEaseOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    return Sqrt((T(2) - value) * value);
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T CircularEaseInOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");

    if (value < T(0.5))
      return T(0.5) * (One<T>() - Sqrt(One<T>() - T(4) * (value * value)));
    else
      return T(0.5) * (Sqrt(-((T(2) * value) - T(3)) * ((T(2) * value) - One<T>())) + One<T>());
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T ExponentialEaseIn(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");

    if (value <= Zero<T>())
      return value;
    else
    {
      const T complementary = value - One<T>();
      const T two = T(2);
      return Pow(two, complementary * T(10));
    }
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T ExponentialEaseOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");

    if (value >= One<T>())
      return value;
    else
      return One<T>() - Pow(T(2), -T(10) * value);
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T ExponentialEaseInOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");

    if (value < T(0.5))
      return T(0.5) * Pow(T(2), (T(20) * value) - T(10));
    else
      return -T(0.5) * Pow(T(2), (-T(20) * value) + T(10)) + One<T>();
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T ElasticEaseIn(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    return Sin(T(13) * HalfPi<T>() * value) * Pow(T(2), T(10) * (value - One<T>()));
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T ElasticEaseOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    return Sin(-T(13) * HalfPi<T>() * (value + One<T>())) * Pow(T(2), -T(10) * value) + One<T>();
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T ElasticEaseInOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");

    if (value < T(0.5))
      return T(0.5) * Sin(T(13) * HalfPi<T>() * (T(2) * value))
             * Pow(T(2), T(10) * ((T(2) * value) - One<T>()));
    else
      return T(0.5)
             * (Sin(-T(13) * HalfPi<T>() * ((T(2) * value - One<T>()) + One<T>()))
                  * Pow(T(2), -T(10) * (T(2) * value - One<T>()))
                + T(2));
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T BackEaseIn(const T value, const T o) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    T z = ((o + One<T>()) * value) - o;
    return (value * value * z);
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T BackEaseOut(const T value, const T o) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");

    T n = value - One<T>();
    T z = ((o + One<T>()) * n) + o;
    return (n * n * z) + One<T>();
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T BackEaseInOut(const T value, const T o) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");

    T s = o * T(1.525);
    T x = T(0.5);
    T n = value / T(0.5);

    if (n < T(1))
    {
      T z = ((s + T(1)) * n) - s;
      T m = n * n * z;
      return x * m;
    }
    else
    {
      n -= T(2);
      T z = ((s + T(1)) * n) + s;
      T m = (n * n * z) + T(2);
      return x * m;
    }
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T BackEaseIn(const T value) noexcept
  {
    return BackEaseIn(value, T(1.70158));
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T BackEaseOut(const T value) noexcept
  {
    return BackEaseOut(value, T(1.70158));
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T BackEaseInOut(const T value) noexcept
  {
    return BackEaseInOut(value, T(1.70158));
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T BounceEaseOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");

    if (value < T(4.0 / 11.0))
      return (T(121) * value * value) / T(16);
    else if (value < T(8.0 / 11.0))
      return (T(363.0 / 40.0) * value * value) - (T(99.0 / 10.0) * value) + T(17.0 / 5.0);
    else if (value < T(9.0 / 10.0))
      return (T(4356.0 / 361.0) * value * value) - (T(35442.0 / 1805.0) * value) + T(16061.0 / 1805.0);
    else
      return (T(54.0 / 5.0) * value * value) - (T(513.0 / 25.0) * value) + T(268.0 / 25.0);
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T BounceEaseIn(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");
    return One<T>() - BounceEaseOut(One<T>() - value);
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T BounceEaseInOut(const T value) noexcept
  {
    assert(value >= Zero<T>() && value <= One<T>(), "Value out of bounds [0,1]");

    if (value < T(0.5))
      return T(0.5) * (One<T>() - BounceEaseOut(One<T>() - value * T(2)));
    else
      return T(0.5) * BounceEaseOut(value * T(2) - One<T>()) + T(0.5);
  }
}
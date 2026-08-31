#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <cmath>
#include <limits>
#include <numbers>

namespace Krys
{
  template <typename T>
  using Limits = std::numeric_limits<T>;

  template <Number T>
  KRYS_NODISCARD constexpr bool IsNaN(T value) noexcept
  {
    if constexpr (FloatingPoint<T>)
    {
      return value != value;
    }

    return false;
  }

  KRYS_NODISCARD constexpr uint16 FastMultiplyBy255(uint16 value) noexcept
  {
    return (value << 8) - value;
  }

  /// @brief While this is an approximate algorithm for division by 255, it gives perfectly accurate results
  /// for 16-bit values.
  KRYS_NODISCARD constexpr uint16 FastDivideBy255(uint16 value) noexcept
  {
    uint16 approximation = value >> 8;
    uint16 remainder = value - (approximation * 255) + 1;
    return approximation + (remainder >> 8);
  }

#pragma region Angle Utils

  constexpr double radiansPerDegreeDouble = std::numbers::pi / 180.0;
  constexpr double degreesPerRadianDouble = 180.0 / std::numbers::pi;
  constexpr double gradientsPerDegreeDouble = 400.0 / 360.0;
  constexpr double degreesPerGradientDouble = 360.0 / 400.0;
  constexpr double turnsPerDegreeDouble = 1.0 / 360.0;
  constexpr double degreesPerTurnDouble = 360.0;
  constexpr double radiansPerTurnDouble = 2.0 * std::numbers::pi;

  KRYS_NODISCARD constexpr double deg2rad(double d) noexcept
  {
    return d * radiansPerDegreeDouble;
  }

  KRYS_NODISCARD constexpr double rad2deg(double r) noexcept
  {
    return r * degreesPerRadianDouble;
  }

  KRYS_NODISCARD constexpr double deg2grad(double d) noexcept
  {
    return d * gradientsPerDegreeDouble;
  }

  KRYS_NODISCARD constexpr double grad2deg(double g) noexcept
  {
    return g * degreesPerGradientDouble;
  }

  KRYS_NODISCARD constexpr double deg2turn(double d) noexcept
  {
    return d * turnsPerDegreeDouble;
  }

  KRYS_NODISCARD constexpr double turn2deg(double t) noexcept
  {
    return t * degreesPerTurnDouble;
  }

  // Note that these differ from the casting the double values above in their rounding errors.
  constexpr float radiansPerDegreeFloat = std::numbers::pi_v<float> / 180.0f;
  constexpr float degreesPerRadianFloat = 180.0f / std::numbers::pi_v<float>;
  constexpr float gradientsPerDegreeFloat = 400.0f / 360.0f;
  constexpr float degreesPerGradientFloat = 360.0f / 400.0f;
  constexpr float turnsPerDegreeFloat = 1.0f / 360.0f;
  constexpr float degreesPerTurnFloat = 360.0f;
  constexpr float radiansPerTurnFloat = 2.0f * std::numbers::pi_v<float>;

  KRYS_NODISCARD constexpr float deg2rad(float d) noexcept
  {
    return d * radiansPerDegreeFloat;
  }

  KRYS_NODISCARD constexpr float rad2deg(float r) noexcept
  {
    return r * degreesPerRadianFloat;
  }

  KRYS_NODISCARD constexpr float deg2grad(float d) noexcept
  {
    return d * gradientsPerDegreeFloat;
  }

  KRYS_NODISCARD constexpr float grad2deg(float g) noexcept
  {
    return g * degreesPerGradientFloat;
  }

  KRYS_NODISCARD constexpr float deg2turn(float d) noexcept
  {
    return d * turnsPerDegreeFloat;
  }

  KRYS_NODISCARD constexpr float turn2deg(float t) noexcept
  {
    return t * degreesPerTurnFloat;
  }

  // Treat these as conversions through the canonical unit for angles, which is degrees.
  KRYS_NODISCARD constexpr double rad2grad(double r) noexcept
  {
    return deg2grad(rad2deg(r));
  }

  KRYS_NODISCARD constexpr double grad2rad(double g) noexcept
  {
    return deg2rad(grad2deg(g));
  }

  KRYS_NODISCARD constexpr double turn2grad(double t) noexcept
  {
    return deg2grad(turn2deg(t));
  }

  KRYS_NODISCARD constexpr double grad2turn(double g) noexcept
  {
    return deg2turn(grad2deg(g));
  }

  KRYS_NODISCARD constexpr double turn2rad(double t) noexcept
  {
    return deg2rad(turn2deg(t));
  }

  KRYS_NODISCARD constexpr double rad2turn(double r) noexcept
  {
    return deg2turn(rad2deg(r));
  }

  KRYS_NODISCARD constexpr float rad2grad(float r) noexcept
  {
    return deg2grad(rad2deg(r));
  }

  KRYS_NODISCARD constexpr float grad2rad(float g) noexcept
  {
    return deg2rad(grad2deg(g));
  }

  KRYS_NODISCARD constexpr float turn2grad(float t) noexcept
  {
    return deg2grad(turn2deg(t));
  }

  KRYS_NODISCARD constexpr float grad2turn(float g) noexcept
  {
    return deg2turn(grad2deg(g));
  }

  KRYS_NODISCARD constexpr float turn2rad(float t) noexcept
  {
    return deg2rad(turn2deg(t));
  }

  KRYS_NODISCARD constexpr float rad2turn(float r) noexcept
  {
    return deg2turn(rad2deg(r));
  }

#pragma endregion

#pragma region ClampTo

  KRYS_NODISCARD inline double RoundTowardsPositiveInfinity(double value) noexcept
  {
    return std::floor(value + 0.5);
  }

  KRYS_NODISCARD inline float RoundTowardsPositiveInfinity(float value) noexcept
  {
    return std::floor(value + 0.5f);
  }

  // Limits<T>::min() returns the smallest positive value for floating point types
  template <typename T>
  KRYS_NODISCARD consteval T DefaultMinimumForClamp() noexcept
  {
    return Limits<T>::min();
  }

  template <>
  KRYS_NODISCARD consteval float DefaultMinimumForClamp() noexcept
  {
    return -Limits<float>::max();
  }

  template <>
  KRYS_NODISCARD consteval double DefaultMinimumForClamp() noexcept
  {
    return -Limits<double>::max();
  }

  template <typename T>
  KRYS_NODISCARD consteval T DefaultMaximumForClamp() noexcept
  {
    return Limits<T>::max();
  }

  // Same type in and out.
  template <typename TargetType, typename SourceType>
  requires SameType<TargetType, SourceType>
  KRYS_NODISCARD constexpr TargetType ClampTo(SourceType value,
                                              TargetType min = DefaultMinimumForClamp<TargetType>(),
                                              TargetType max = DefaultMaximumForClamp<TargetType>()) noexcept
  {
    if (value >= max)
    {
      return max;
    }

    if (value <= min)
    {
      return min;
    }

    return value;
  }

  // Floating point source.
  template <typename TargetType, typename SourceType>
  requires(!SameType<TargetType, SourceType> && FloatingPoint<SourceType>
           && !(FloatingPoint<TargetType> && sizeof(TargetType) > sizeof(SourceType)))
  KRYS_NODISCARD constexpr TargetType ClampTo(SourceType value,
                                              TargetType min = DefaultMinimumForClamp<TargetType>(),
                                              TargetType max = DefaultMaximumForClamp<TargetType>()) noexcept
  {
    if (value >= static_cast<SourceType>(max))
    {
      return max;
    }

    // This will return min if value is NaN.
    if (!(value > static_cast<SourceType>(min)))
    {
      return min;
    }

    return static_cast<TargetType>(value);
  }

  template <typename TargetType, typename SourceType>
  requires(!SameType<TargetType, SourceType> && FloatingPoint<SourceType> && FloatingPoint<TargetType>
           && sizeof(TargetType) > sizeof(SourceType))
  KRYS_NODISCARD constexpr TargetType ClampTo(SourceType value,
                                              TargetType min = DefaultMinimumForClamp<TargetType>(),
                                              TargetType max = DefaultMaximumForClamp<TargetType>()) noexcept
  {
    TargetType convertedValue = static_cast<TargetType>(value);
    if (convertedValue >= max)
    {
      return max;
    }

    if (convertedValue <= min)
    {
      return min;
    }

    return convertedValue;
  }

  // Source and Target have the same sign and Source is larger or equal to Target
  template <typename TargetType, typename SourceType>
  requires(!SameType<TargetType, SourceType> && Integral<SourceType> && Integral<TargetType>
           && SignedIntegral<TargetType> == SignedIntegral<SourceType>
           && sizeof(SourceType) >= sizeof(TargetType))
  KRYS_NODISCARD constexpr TargetType ClampTo(SourceType value,
                                              TargetType min = DefaultMinimumForClamp<TargetType>(),
                                              TargetType max = DefaultMaximumForClamp<TargetType>()) noexcept
  {
    if (value >= static_cast<SourceType>(max))
    {
      return max;
    }

    if (value <= static_cast<SourceType>(min))
    {
      return min;
    }

    return static_cast<TargetType>(value);
  }

  // Clamping a unsigned integer to the max signed value.
  template <typename TargetType, typename SourceType>
  requires(!SameType<TargetType, SourceType> && UnsignedIntegral<SourceType> && SignedIntegral<TargetType>
           && sizeof(SourceType) >= sizeof(TargetType))
  KRYS_NODISCARD constexpr TargetType ClampTo(SourceType value) noexcept
  {
    TargetType max = Limits<TargetType>::max();
    if (value >= static_cast<SourceType>(max))
    {
      return max;
    }

    return static_cast<TargetType>(value);
  }

  // Clamping a signed integer into a valid unsigned integer.
  template <typename TargetType, typename SourceType>
  requires(!SameType<TargetType, SourceType> && UnsignedIntegral<TargetType> && SignedIntegral<SourceType>
           && sizeof(SourceType) == sizeof(TargetType))
  KRYS_NODISCARD constexpr TargetType ClampTo(SourceType value) noexcept
  {
    if (value < 0)
    {
      return 0;
    }

    return static_cast<TargetType>(value);
  }

  template <typename TargetType, typename SourceType>
  requires(!SameType<TargetType, SourceType> && UnsignedIntegral<TargetType> && SignedIntegral<SourceType>
           && sizeof(SourceType) > sizeof(TargetType))
  KRYS_NODISCARD constexpr TargetType ClampTo(SourceType value,
                                              TargetType min = DefaultMinimumForClamp<TargetType>(),
                                              TargetType max = DefaultMaximumForClamp<TargetType>()) noexcept
  {
    if (value >= static_cast<SourceType>(max))
    {
      return max;
    }

    if (value <= static_cast<SourceType>(min))
    {
      return min;
    }

    return static_cast<TargetType>(value);
  }

#pragma endregion
}
#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include <cmath>
#include <limits>

namespace Krys
{
  KRYS_NODISCARD inline double RoundTowardsPositiveInfinity(double value) noexcept
  {
    return std::floor(value + 0.5);
  }

  KRYS_NODISCARD inline float RoundTowardsPositiveInfinity(float value) noexcept
  {
    return std::floor(value + 0.5f);
  }

  // std::numeric_limits<T>::min() returns the smallest positive value for floating point types
  template <typename T>
  KRYS_NODISCARD consteval T DefaultMinimumForClamp() noexcept
  {
    return std::numeric_limits<T>::min();
  }

  template <>
  KRYS_NODISCARD consteval float DefaultMinimumForClamp() noexcept
  {
    return -std::numeric_limits<float>::max();
  }

  template <>
  KRYS_NODISCARD consteval double DefaultMinimumForClamp() noexcept
  {
    return -std::numeric_limits<double>::max();
  }

  template <typename T>
  KRYS_NODISCARD consteval T DefaultMaximumForClamp() noexcept
  {
    return std::numeric_limits<T>::max();
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
    TargetType max = std::numeric_limits<TargetType>::max();
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
}
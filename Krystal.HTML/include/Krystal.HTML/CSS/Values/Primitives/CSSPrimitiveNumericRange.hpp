#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace Krys::HTML
{
#pragma region Clamp Utils

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

#pragma endregion

  /// @brief Options to indicate how the range should be interpreted.
  enum class RangeClampOptions : uint8
  {
    /// @brief Indicates that at parse time, out of range values invalidate the parse.
    /// Out of range values at style building always clamp.
    Default,

    /// @brief Indicates that at parse time, an out of range lower value should clamp
    /// instead of invalidating the parse. An out of range upper value will still invalidate
    /// the parse. Out of range values at style building always clamp.
    ClampLower,

    /// @brief Indicates that at parse time, an out of range upper value should clamp
    /// instead of invalidating the parse. An out of range lower value will still invalidate
    /// the parse. Out of range values at style building always clamp.
    ClampUpper,

    /// @brief Indicates that at parse time, an out of range lower or upper value should
    /// clamp instead of invalidating the parse. Out of range values at style building
    /// always clamp.
    ClampBoth
  };

  /// @brief Representation for `CSS bracketed range notation`. Represents a closed range between (and
  /// including) `min` and `max`.
  /// @see https://drafts.csswg.org/css-values-4/#numeric-ranges
  struct CSSRange
  {
    constexpr static double Inf = std::numeric_limits<double>::infinity();

    double Min {-Inf};
    double Max {Inf};
    RangeClampOptions ClampOptions {RangeClampOptions::Default};

    constexpr CSSRange(double min, double max,
                    RangeClampOptions clampOptions = RangeClampOptions::Default) noexcept
        : Min(min), Max(max), ClampOptions(clampOptions)
    {
    }

    constexpr bool operator==(const CSSRange &) const = default;
  };

  /// @brief Constant value for `[−∞,∞]`.
  constexpr auto All = CSSRange {-CSSRange::Inf, CSSRange::Inf};

  /// @brief Constant value for `[0,∞]`.
  constexpr auto NonNegative = CSSRange {0, CSSRange::Inf};

  /// @brief Constant value for `[1,∞]`.
  constexpr auto Positive = CSSRange {1, CSSRange::Inf};

  /// @brief Constant value for `[0,1]`.
  constexpr auto ClosedUnitRange = CSSRange {0, 1};

  /// @brief Constant value for `[0,1(clamp upper)]`.
  constexpr auto ClosedUnitRangeClampUpper = CSSRange {0, 1, RangeClampOptions::ClampUpper};

  /// @brief Constant value for `[0,1(clamp both)]`.
  constexpr auto ClosedUnitRangeClampBoth = CSSRange {0, 1, RangeClampOptions::ClampBoth};

  /// @brief Constant value for `[0,100]`.
  constexpr auto ClosedPercentageRange = CSSRange {0, 100};

  /// @brief Constant value for `[0,100(clamp upper)]`.
  constexpr auto ClosedPercentageRangeClampUpper = CSSRange {0, 100, RangeClampOptions::ClampUpper};

  /// @brief Clamps a floating point value to within `range`.
  template <CSSRange range, FloatingPoint T, typename U>
  KRYS_NODISCARD constexpr T ClampToRange(U value) noexcept
  {
    return ClampTo<T>(value, std::max<T>(range.Min, -std::numeric_limits<T>::max()),
                      std::min<T>(range.Max, std::numeric_limits<T>::max()));
  }

  /// @brief Clamps a floating point value to within `range` and within additional provided range.
  template <CSSRange range, FloatingPoint T, typename U>
  KRYS_NODISCARD constexpr T ClampToRange(U value, T additionalMinimum, T additionalMaximum) noexcept
  {
    return ClampTo<T>(value,
                      std::max<T>(std::max<T>(range.Min, -std::numeric_limits<T>::max()), additionalMinimum),
                      std::min<T>(std::min<T>(range.Max, std::numeric_limits<T>::max()), additionalMaximum));
  }

  /// @brief Clamps an unsigned integral value to within `range`.
  template <CSSRange range, UnsignedIntegral T, typename U>
  KRYS_NODISCARD constexpr T ClampToRange(U value) noexcept
  {
    static_assert(range.Min >= 0);

    if constexpr (range.Max == CSSRange::Inf)
    {
      return ClampTo<T>(value, range.Min, std::numeric_limits<T>::max());
    }
    else
    {
      return ClampTo<T>(value, range.Min, std::min<T>(range.Max, std::numeric_limits<T>::max()));
    }
  }

  /// @brief Clamps a signed integral value to within `range`.
  template <CSSRange range, SignedIntegral T, typename U>
  KRYS_NODISCARD constexpr T ClampToRange(U value) noexcept
  {
    if constexpr (range.Min == -CSSRange::Inf && range.Max == CSSRange::Inf)
    {
      return ClampTo<T>(value, std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    }
    else if constexpr (range.Min == -CSSRange::Inf)
    {
      return ClampTo<T>(value, std::numeric_limits<T>::min(),
                        std::min<T>(range.Max, std::numeric_limits<T>::max()));
    }
    else if constexpr (range.Max == CSSRange::Inf)
    {
      return ClampTo<T>(value, std::max<T>(range.Min, std::numeric_limits<T>::min()),
                        std::numeric_limits<T>::max());
    }
    else
    {
      return ClampTo<T>(value, std::max<T>(range.Min, std::numeric_limits<T>::min()),
                        std::min<T>(range.Max, std::numeric_limits<T>::max()));
    }
  }

  /// @brief Checks if a floating point value is within `range`.
  template <CSSRange range, FloatingPoint T>
  KRYS_NODISCARD constexpr bool IsWithinRange(T value) noexcept
  {
    return !std::isnan(value) && value >= std::max<T>(range.Min, -std::numeric_limits<T>::max())
           && value <= std::min<T>(range.Max, std::numeric_limits<T>::max());
  }

  /// @brief Checks if a signed integral value is within `range`.
  template <CSSRange range, SignedIntegral T>
  KRYS_NODISCARD constexpr bool IsWithinRange(T value) noexcept
  {
    if constexpr (range.Min == -CSSRange::Inf && range.Max == CSSRange::Inf)
    {
      return value >= std::numeric_limits<T>::min() && value <= std::numeric_limits<T>::max();
    }
    else if constexpr (range.Min == -CSSRange::Inf)
    {
      return value >= std::numeric_limits<T>::min()
             && value <= std::min<T>(range.Max, std::numeric_limits<T>::max());
    }
    else if constexpr (range.Max == CSSRange::Inf)
    {
      return value >= std::max<T>(range.Min, std::numeric_limits<T>::min())
             && value <= std::numeric_limits<T>::max();
    }
    else
    {
      return value >= std::max<T>(range.Min, std::numeric_limits<T>::min())
             && value <= std::min<T>(range.Max, std::numeric_limits<T>::max());
    }
  }

  /// @brief Checks if an unsigned integral value is within `range`.
  template <CSSRange range, UnsignedIntegral T>
  KRYS_NODISCARD constexpr bool IsWithinRange(T value) noexcept
  {
    static_assert(range.Min >= 0);

    if constexpr (range.Max == CSSRange::Inf)
    {
      return value >= std::max<T>(range.Min, std::numeric_limits<T>::min())
             && value <= std::numeric_limits<T>::max();
    }
    else
    {
      return value >= std::max<T>(range.Min, std::numeric_limits<T>::min())
             && value <= std::min<T>(range.Max, std::numeric_limits<T>::max());
    }
  }
}
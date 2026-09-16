#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <algorithm>

namespace krys::boo::css
{
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

  // Options to indicate how the primitive should consider its value with regards to zoom.
  // NOTE: This option is only meaningful for Style::Length`.
  // FIXME: These options are temporary while `zoom` is moving from style building time to use time.
  enum class RangeZoomOptions : bool
  {
    // `Default` indicates the value held in the primitive has had zoom applied to it.
    Default,

    // `Unzoomed` indicates the value held in the primitive has NOT had zoom applied to it.
    Unzoomed
  };

  /// @brief Representation for `CSS bracketed range notation`. Represents a closed range between (and
  /// including) `min` and `max`.
  /// @see https://drafts.csswg.org/css-values-4/#numeric-ranges
  struct Range
  {
    constexpr static double Inf = std::numeric_limits<double>::infinity();

    double Min {-Inf};
    double Max {Inf};
    RangeClampOptions ClampOptions {RangeClampOptions::Default};
    RangeZoomOptions ZoomOptions {RangeZoomOptions::Default};

    constexpr Range(double min, double max, RangeClampOptions clampOptions = RangeClampOptions::Default,
                    RangeZoomOptions zoomOptions = RangeZoomOptions::Default) noexcept
        : Min(min), Max(max), ClampOptions(clampOptions), ZoomOptions(zoomOptions)
    {
    }

    constexpr bool operator==(const Range &) const = default;
  };

  /// @brief Constant value for `[−∞,∞]`.
  constexpr auto All = Range {-Range::Inf, Range::Inf};
  constexpr auto AllUnzoomed =
    Range {-Range::Inf, Range::Inf, RangeClampOptions::Default, RangeZoomOptions::Unzoomed};

  /// @brief Constant value for `[0,∞]`.
  constexpr auto NonNegative = Range {0, Range::Inf};
  constexpr auto NonNegativeUnzoomed =
    Range {0, Range::Inf, RangeClampOptions::Default, RangeZoomOptions::Unzoomed};

  /// @brief Constant value for `[1,∞]`.
  constexpr auto Positive = Range {1, Range::Inf};
  constexpr auto PositiveUnzoomed =
    Range {1, Range::Inf, RangeClampOptions::Default, RangeZoomOptions::Unzoomed};

  /// @brief Constant value for `[0,1]`.
  constexpr auto ClosedUnitRange = Range {0, 1};
  constexpr auto ClosedUnitRangeUnzoomed =
    Range {0, 1, RangeClampOptions::Default, RangeZoomOptions::Unzoomed};

  /// @brief Constant value for `[0,1(clamp upper)]`.
  constexpr auto ClosedUnitRangeClampUpper = Range {0, 1, RangeClampOptions::ClampUpper};
  constexpr auto ClosedUnitRangeClampUpperUnzoomed =
    Range {0, 1, RangeClampOptions::ClampUpper, RangeZoomOptions::Unzoomed};

  /// @brief Constant value for `[0,1(clamp both)]`.
  constexpr auto ClosedUnitRangeClampBoth = Range {0, 1, RangeClampOptions::ClampBoth};
  constexpr auto ClosedUnitRangeClampBothUnzoomed =
    Range {0, 1, RangeClampOptions::ClampBoth, RangeZoomOptions::Unzoomed};

  /// @brief Constant value for `[0,100]`.
  constexpr auto ClosedPercentageRange = Range {0, 100};
  constexpr auto ClosedPercentageRangeUnzoomed =
    Range {0, 100, RangeClampOptions::Default, RangeZoomOptions::Unzoomed};

  /// @brief Constant value for `[0,100(clamp upper)]`.
  constexpr auto ClosedPercentageRangeClampUpper = Range {0, 100, RangeClampOptions::ClampUpper};
  constexpr auto ClosedPercentageRangeClampUpperUnzoomed =
    Range {0, 100, RangeClampOptions::ClampUpper, RangeZoomOptions::Unzoomed};

  /// @brief Clamps a floating point value to within `range`.
  template <Range range, FloatingPoint T, typename U>
  KRYS_NODISCARD constexpr T ClampToRange(U value) noexcept
  {
    return ClampTo<T>(value, std::max<T>(range.Min, -std::numeric_limits<T>::max()),
                      std::min<T>(range.Max, std::numeric_limits<T>::max()));
  }

  /// @brief Clamps a floating point value to within `range` and within additional provided range.
  template <Range range, FloatingPoint T, typename U>
  KRYS_NODISCARD constexpr T ClampToRange(U value, T additionalMinimum, T additionalMaximum) noexcept
  {
    return ClampTo<T>(value,
                      std::max<T>(std::max<T>(range.Min, -std::numeric_limits<T>::max()), additionalMinimum),
                      std::min<T>(std::min<T>(range.Max, std::numeric_limits<T>::max()), additionalMaximum));
  }

  /// @brief Clamps an unsigned integral value to within `range`.
  template <Range range, UnsignedIntegral T, typename U>
  KRYS_NODISCARD constexpr T ClampToRange(U value) noexcept
  {
    static_assert(range.Min >= 0);

    if constexpr (range.Max == Range::Inf)
    {
      return ClampTo<T>(value, range.Min, std::numeric_limits<T>::max());
    }
    else
    {
      return ClampTo<T>(value, range.Min, std::min<T>(range.Max, std::numeric_limits<T>::max()));
    }
  }

  /// @brief Clamps a signed integral value to within `range`.
  template <Range range, SignedIntegral T, typename U>
  KRYS_NODISCARD constexpr T ClampToRange(U value) noexcept
  {
    if constexpr (range.Min == -Range::Inf && range.Max == Range::Inf)
    {
      return ClampTo<T>(value, std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    }
    else if constexpr (range.Min == -Range::Inf)
    {
      return ClampTo<T>(value, std::numeric_limits<T>::min(),
                        std::min<T>(range.Max, std::numeric_limits<T>::max()));
    }
    else if constexpr (range.Max == Range::Inf)
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
  template <Range range, FloatingPoint T>
  KRYS_NODISCARD constexpr bool IsWithinRange(T value) noexcept
  {
    return !std::isnan(value) && value >= std::max<T>(range.Min, -std::numeric_limits<T>::max())
           && value <= std::min<T>(range.Max, std::numeric_limits<T>::max());
  }

  /// @brief Checks if a signed integral value is within `range`.
  template <Range range, SignedIntegral T>
  KRYS_NODISCARD constexpr bool IsWithinRange(T value) noexcept
  {
    if constexpr (range.Min == -Range::Inf && range.Max == Range::Inf)
    {
      return value >= std::numeric_limits<T>::min() && value <= std::numeric_limits<T>::max();
    }
    else if constexpr (range.Min == -Range::Inf)
    {
      return value >= std::numeric_limits<T>::min()
             && value <= std::min<T>(range.Max, std::numeric_limits<T>::max());
    }
    else if constexpr (range.Max == Range::Inf)
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
  template <Range range, UnsignedIntegral T>
  KRYS_NODISCARD constexpr bool IsWithinRange(T value) noexcept
  {
    static_assert(range.Min >= 0);

    if constexpr (range.Max == Range::Inf)
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
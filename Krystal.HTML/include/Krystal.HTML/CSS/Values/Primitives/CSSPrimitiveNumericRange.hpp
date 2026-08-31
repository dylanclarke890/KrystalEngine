#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Maths.hpp"
#include <algorithm>

namespace Krys::HTML
{
  /// @brief Options to indicate how the tokens should be interpreted.
  enum class RangeClampOptions : uint8
  {
    /// @brief Indicates that at parse time, out of tokens values invalidate the parse.
    /// Out of tokens values at style building always clamp.
    Default,

    /// @brief Indicates that at parse time, an out of tokens lower value should clamp
    /// instead of invalidating the parse. An out of tokens upper value will still invalidate
    /// the parse. Out of tokens values at style building always clamp.
    ClampLower,

    /// @brief Indicates that at parse time, an out of tokens upper value should clamp
    /// instead of invalidating the parse. An out of tokens lower value will still invalidate
    /// the parse. Out of tokens values at style building always clamp.
    ClampUpper,

    /// @brief Indicates that at parse time, an out of tokens lower or upper value should
    /// clamp instead of invalidating the parse. Out of tokens values at style building
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

  /// @brief Representation for `CSS bracketed tokens notation`. Represents a closed tokens between (and
  /// including) `min` and `max`.
  /// @see https://drafts.csswg.org/css-values-4/#numeric-ranges
  struct CSSRange
  {
    constexpr static double Inf = std::numeric_limits<double>::infinity();

    double Min {-Inf};
    double Max {Inf};
    RangeClampOptions ClampOptions {RangeClampOptions::Default};
    RangeZoomOptions ZoomOptions {RangeZoomOptions::Default};

    constexpr CSSRange(double min, double max, RangeClampOptions clampOptions = RangeClampOptions::Default,
                       RangeZoomOptions zoomOptions = RangeZoomOptions::Default) noexcept
        : Min(min), Max(max), ClampOptions(clampOptions), ZoomOptions(zoomOptions)
    {
    }

    constexpr bool operator==(const CSSRange &) const = default;
  };

  /// @brief Constant value for `[−∞,∞]`.
  constexpr auto All = CSSRange {-CSSRange::Inf, CSSRange::Inf};
  constexpr auto AllUnzoomed =
    CSSRange {-CSSRange::Inf, CSSRange::Inf, RangeClampOptions::Default, RangeZoomOptions::Unzoomed};

  /// @brief Constant value for `[0,∞]`.
  constexpr auto NonNegative = CSSRange {0, CSSRange::Inf};
  constexpr auto NonNegativeUnzoomed =
    CSSRange {0, CSSRange::Inf, RangeClampOptions::Default, RangeZoomOptions::Unzoomed};

  /// @brief Constant value for `[1,∞]`.
  constexpr auto Positive = CSSRange {1, CSSRange::Inf};
  constexpr auto PositiveUnzoomed =
    CSSRange {1, CSSRange::Inf, RangeClampOptions::Default, RangeZoomOptions::Unzoomed};

  /// @brief Constant value for `[0,1]`.
  constexpr auto ClosedUnitRange = CSSRange {0, 1};
  constexpr auto ClosedUnitRangeUnzoomed =
    CSSRange {0, 1, RangeClampOptions::Default, RangeZoomOptions::Unzoomed};

  /// @brief Constant value for `[0,1(clamp upper)]`.
  constexpr auto ClosedUnitRangeClampUpper = CSSRange {0, 1, RangeClampOptions::ClampUpper};
  constexpr auto ClosedUnitRangeClampUpperUnzoomed =
    CSSRange {0, 1, RangeClampOptions::ClampUpper, RangeZoomOptions::Unzoomed};

  /// @brief Constant value for `[0,1(clamp both)]`.
  constexpr auto ClosedUnitRangeClampBoth = CSSRange {0, 1, RangeClampOptions::ClampBoth};
  constexpr auto ClosedUnitRangeClampBothUnzoomed =
    CSSRange {0, 1, RangeClampOptions::ClampBoth, RangeZoomOptions::Unzoomed};

  /// @brief Constant value for `[0,100]`.
  constexpr auto ClosedPercentageRange = CSSRange {0, 100};
  constexpr auto ClosedPercentageRangeUnzoomed =
    CSSRange {0, 100, RangeClampOptions::Default, RangeZoomOptions::Unzoomed};

  /// @brief Constant value for `[0,100(clamp upper)]`.
  constexpr auto ClosedPercentageRangeClampUpper = CSSRange {0, 100, RangeClampOptions::ClampUpper};
  constexpr auto ClosedPercentageRangeClampUpperUnzoomed =
    CSSRange {0, 100, RangeClampOptions::ClampUpper, RangeZoomOptions::Unzoomed};

  /// @brief Clamps a floating point value to within `tokens`.
  template <CSSRange tokens, FloatingPoint T, typename U>
  KRYS_NODISCARD constexpr T ClampToRange(U value) noexcept
  {
    return ClampTo<T>(value, std::max<T>(tokens.Min, -std::numeric_limits<T>::max()),
                      std::min<T>(tokens.Max, std::numeric_limits<T>::max()));
  }

  /// @brief Clamps a floating point value to within `tokens` and within additional provided tokens.
  template <CSSRange tokens, FloatingPoint T, typename U>
  KRYS_NODISCARD constexpr T ClampToRange(U value, T additionalMinimum, T additionalMaximum) noexcept
  {
    return ClampTo<T>(value,
                      std::max<T>(std::max<T>(tokens.Min, -std::numeric_limits<T>::max()), additionalMinimum),
                      std::min<T>(std::min<T>(tokens.Max, std::numeric_limits<T>::max()), additionalMaximum));
  }

  /// @brief Clamps an unsigned integral value to within `tokens`.
  template <CSSRange tokens, UnsignedIntegral T, typename U>
  KRYS_NODISCARD constexpr T ClampToRange(U value) noexcept
  {
    static_assert(tokens.Min >= 0);

    if constexpr (tokens.Max == CSSRange::Inf)
    {
      return ClampTo<T>(value, tokens.Min, std::numeric_limits<T>::max());
    }
    else
    {
      return ClampTo<T>(value, tokens.Min, std::min<T>(tokens.Max, std::numeric_limits<T>::max()));
    }
  }

  /// @brief Clamps a signed integral value to within `tokens`.
  template <CSSRange tokens, SignedIntegral T, typename U>
  KRYS_NODISCARD constexpr T ClampToRange(U value) noexcept
  {
    if constexpr (tokens.Min == -CSSRange::Inf && tokens.Max == CSSRange::Inf)
    {
      return ClampTo<T>(value, std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    }
    else if constexpr (tokens.Min == -CSSRange::Inf)
    {
      return ClampTo<T>(value, std::numeric_limits<T>::min(),
                        std::min<T>(tokens.Max, std::numeric_limits<T>::max()));
    }
    else if constexpr (tokens.Max == CSSRange::Inf)
    {
      return ClampTo<T>(value, std::max<T>(tokens.Min, std::numeric_limits<T>::min()),
                        std::numeric_limits<T>::max());
    }
    else
    {
      return ClampTo<T>(value, std::max<T>(tokens.Min, std::numeric_limits<T>::min()),
                        std::min<T>(tokens.Max, std::numeric_limits<T>::max()));
    }
  }

  /// @brief Checks if a floating point value is within `tokens`.
  template <CSSRange tokens, FloatingPoint T>
  KRYS_NODISCARD constexpr bool IsWithinRange(T value) noexcept
  {
    return !std::isnan(value) && value >= std::max<T>(tokens.Min, -std::numeric_limits<T>::max())
           && value <= std::min<T>(tokens.Max, std::numeric_limits<T>::max());
  }

  /// @brief Checks if a signed integral value is within `tokens`.
  template <CSSRange tokens, SignedIntegral T>
  KRYS_NODISCARD constexpr bool IsWithinRange(T value) noexcept
  {
    if constexpr (tokens.Min == -CSSRange::Inf && tokens.Max == CSSRange::Inf)
    {
      return value >= std::numeric_limits<T>::min() && value <= std::numeric_limits<T>::max();
    }
    else if constexpr (tokens.Min == -CSSRange::Inf)
    {
      return value >= std::numeric_limits<T>::min()
             && value <= std::min<T>(tokens.Max, std::numeric_limits<T>::max());
    }
    else if constexpr (tokens.Max == CSSRange::Inf)
    {
      return value >= std::max<T>(tokens.Min, std::numeric_limits<T>::min())
             && value <= std::numeric_limits<T>::max();
    }
    else
    {
      return value >= std::max<T>(tokens.Min, std::numeric_limits<T>::min())
             && value <= std::min<T>(tokens.Max, std::numeric_limits<T>::max());
    }
  }

  /// @brief Checks if an unsigned integral value is within `tokens`.
  template <CSSRange tokens, UnsignedIntegral T>
  KRYS_NODISCARD constexpr bool IsWithinRange(T value) noexcept
  {
    static_assert(tokens.Min >= 0);

    if constexpr (tokens.Max == CSSRange::Inf)
    {
      return value >= std::max<T>(tokens.Min, std::numeric_limits<T>::min())
             && value <= std::numeric_limits<T>::max();
    }
    else
    {
      return value >= std::max<T>(tokens.Min, std::numeric_limits<T>::min())
             && value <= std::min<T>(tokens.Max, std::numeric_limits<T>::max());
    }
  }
}
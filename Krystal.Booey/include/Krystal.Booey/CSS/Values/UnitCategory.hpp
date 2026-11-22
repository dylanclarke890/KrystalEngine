#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  /// @brief The kind of quantity a unit measures (`UnitCategoryForType`), which says how it converts: units
  /// of one category convert to each other, except relative lengths, which need a context to resolve.
  enum class UnitCategory : uint8
  {
    /// @brief A bare number or integer.
    Number,
    /// @brief A percentage.
    Percent,
    /// @brief `px`, `cm`, `mm`, `in`, `pt`, `pc` and `Q`.
    AbsoluteLength,
    /// @brief Lengths relative to a font: `em`, `ex`, `cap`, `ch`, `ic`, `lh` and their root forms (`rem`).
    FontRelativeLength,
    /// @brief Lengths relative to the viewport: `vw`, `vh`, `vi`, `vb`, `vmin`, `vmax` and their small, large
    /// and dynamic forms.
    ViewportPercentageLength,
    /// @brief `deg`, `rad`, `grad` and `turn`.
    Angle,
    /// @brief `s` and `ms`.
    Time,
    /// @brief `Hz` and `kHz`.
    Frequency,
    /// @brief `dppx`, `x`, `dpi` and `dpcm`.
    Resolution,
    /// @brief `fr`.
    Flex,
    /// @brief Everything else, such as container query lengths, `calc()` and `attr()`.
    Other,
  };

  consteval UnitCategory MaxEnumValue(UnitCategory) noexcept
  {
    return UnitCategory::Other;
  }
}

#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericCategory.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  /// @brief The unit of a numeric CSS value. Each unit's facts are in `UnitInfoFor`, and everything else here
  /// is derived from them.
  /// @see https://drafts.csswg.org/css-values-4/#numeric-types
  enum class UnitType : uint8
  {
    // An unrecognised unit, or none.
    Unknown,

    Number,
    Integer,
    Percentage,

    // Absolute lengths.
    px,
    cm,
    mm,
    Q,
    in,
    pt,
    pc,

    // Font-relative lengths.
    em,
    ex,
    cap,
    ch,
    ic,
    lh,
    // `em` in quirks mode, written `__qem` and only in the user agent style sheet.
    QuirkyEm,

    // Root-font-relative lengths.
    rem,
    rex,
    rcap,
    rch,
    ric,
    rlh,

    // Viewport-relative lengths: the default, small, large and dynamic viewports.
    vw,
    vh,
    vi,
    vb,
    vmin,
    vmax,
    svw,
    svh,
    svi,
    svb,
    svmin,
    svmax,
    lvw,
    lvh,
    lvi,
    lvb,
    lvmin,
    lvmax,
    dvw,
    dvh,
    dvi,
    dvb,
    dvmin,
    dvmax,

    // Container-relative lengths.
    cqw,
    cqh,
    cqi,
    cqb,
    cqmin,
    cqmax,

    deg,
    rad,
    grad,
    turn,

    s,
    ms,

    hz,
    khz,

    dppx,
    x,
    dpi,
    dpcm,

    fr,
  };

  consteval UnitType MaxEnumValue(UnitType) noexcept
  {
    return UnitType::fr;
  }

  /// @brief The kind of quantity a unit measures, with lengths split by what they're relative to: units of
  /// one category convert to each other, except relative lengths, which need a context to resolve.
  enum class UnitCategory : uint8
  {
    /// @brief A bare number or integer.
    Number,
    /// @brief A percentage.
    Percent,
    /// @brief `px`, `cm`, `mm`, `in`, `pt`, `pc` and `Q`.
    AbsoluteLength,
    /// @brief Lengths relative to a font, the root's included.
    FontRelativeLength,
    /// @brief Lengths relative to a viewport.
    ViewportPercentageLength,
    Angle,
    Time,
    Frequency,
    Resolution,
    Flex,
    /// @brief Everything else: container-relative lengths and `Unknown`.
    Other,
  };

  consteval UnitCategory MaxEnumValue(UnitCategory) noexcept
  {
    return UnitCategory::Other;
  }

  /// @brief What a length is relative to.
  enum class LengthBasis : uint8
  {
    /// @brief Nothing: an absolute length.
    Absolute,
    Font,
    RootFont,
    Viewport,
    Container,
  };

  consteval LengthBasis MaxEnumValue(LengthBasis) noexcept
  {
    return LengthBasis::Container;
  }

#pragma region Fixed ratios between units

  // A CSS inch is always 96 CSS pixels, whatever the device: a high-density screen scales the CSS pixel,
  // which the absolute units follow. https://drafts.csswg.org/css-values-4/#absolute-lengths
  constexpr double pxPerInch = 96;
  constexpr double ptsPerInch = 72;
  constexpr double picasPerInch = 6;
  constexpr double mmPerInch = 25.4;
  constexpr double cmPerInch = 2.54;
  constexpr double qPerInch = mmPerInch * 4.0;
  constexpr double pxPerCm = pxPerInch / cmPerInch;
  constexpr double pxPerMm = pxPerInch / mmPerInch;
  constexpr double pxPerQ = pxPerInch / qPerInch;
  constexpr double pxPerPt = pxPerInch / ptsPerInch;
  constexpr double pxPerPc = pxPerInch / picasPerInch;
  // https://drafts.csswg.org/css-values-4/#resolution
  constexpr double dppxPerX = 1.0;
  constexpr double dppxPerDpi = 1.0 / pxPerInch;
  constexpr double dppxPerDpcm = cmPerInch / pxPerInch;
  // https://drafts.csswg.org/css-values-4/#time
  constexpr double secsPerMs = 1.0 / 1000.0;
  // https://drafts.csswg.org/css-values-4/#frequency
  constexpr double hertzPerKilohertz = 1000.0;

#pragma endregion

  /// @brief What's known about a unit.
  struct UnitInfo
  {
    /// @brief The unit as it's written and serialised, in lowercase; empty for `Unknown`, numbers, integers
    /// and `QuirkyEm`.
    utf8_stringview Name;
    /// @brief The quantity the unit measures; null for `Unknown`.
    Maybe<Category> Quantity;
    /// @brief For a length, what it's relative to.
    LengthBasis Basis {LengthBasis::Absolute};
    /// @brief The fixed factor from the unit to its quantity's canonical unit; null when there isn't one,
    /// such as for relative lengths.
    Maybe<double> ScaleToCanonical;
  };

  /// @brief The facts about @p unit: the one place they're stated.
  KRYS_NODISCARD constexpr UnitInfo UnitInfoFor(UnitType unit) noexcept
  {
    using enum UnitType;
    constexpr auto Length = Category::Length;
    switch (unit)
    {
      case Number:     return {u8"", Category::Number};
      case Integer:    return {u8"", Category::Integer};
      case Percentage: return {u8"%", Category::Percentage};

      case px:         return {u8"px", Length, LengthBasis::Absolute, 1.0};
      case cm:         return {u8"cm", Length, LengthBasis::Absolute, pxPerCm};
      case mm:         return {u8"mm", Length, LengthBasis::Absolute, pxPerMm};
      case Q:          return {u8"q", Length, LengthBasis::Absolute, pxPerQ};
      case in:         return {u8"in", Length, LengthBasis::Absolute, pxPerInch};
      case pt:         return {u8"pt", Length, LengthBasis::Absolute, pxPerPt};
      case pc:         return {u8"pc", Length, LengthBasis::Absolute, pxPerPc};

      case em:         return {u8"em", Length, LengthBasis::Font};
      case ex:         return {u8"ex", Length, LengthBasis::Font};
      case cap:        return {u8"cap", Length, LengthBasis::Font};
      case ch:         return {u8"ch", Length, LengthBasis::Font};
      case ic:         return {u8"ic", Length, LengthBasis::Font};
      case lh:         return {u8"lh", Length, LengthBasis::Font};
      case QuirkyEm:   return {u8"", Length, LengthBasis::Font};

      case rem:        return {u8"rem", Length, LengthBasis::RootFont};
      case rex:        return {u8"rex", Length, LengthBasis::RootFont};
      case rcap:       return {u8"rcap", Length, LengthBasis::RootFont};
      case rch:        return {u8"rch", Length, LengthBasis::RootFont};
      case ric:        return {u8"ric", Length, LengthBasis::RootFont};
      case rlh:        return {u8"rlh", Length, LengthBasis::RootFont};

      case vw:         return {u8"vw", Length, LengthBasis::Viewport};
      case vh:         return {u8"vh", Length, LengthBasis::Viewport};
      case vi:         return {u8"vi", Length, LengthBasis::Viewport};
      case vb:         return {u8"vb", Length, LengthBasis::Viewport};
      case vmin:       return {u8"vmin", Length, LengthBasis::Viewport};
      case vmax:       return {u8"vmax", Length, LengthBasis::Viewport};
      case svw:        return {u8"svw", Length, LengthBasis::Viewport};
      case svh:        return {u8"svh", Length, LengthBasis::Viewport};
      case svi:        return {u8"svi", Length, LengthBasis::Viewport};
      case svb:        return {u8"svb", Length, LengthBasis::Viewport};
      case svmin:      return {u8"svmin", Length, LengthBasis::Viewport};
      case svmax:      return {u8"svmax", Length, LengthBasis::Viewport};
      case lvw:        return {u8"lvw", Length, LengthBasis::Viewport};
      case lvh:        return {u8"lvh", Length, LengthBasis::Viewport};
      case lvi:        return {u8"lvi", Length, LengthBasis::Viewport};
      case lvb:        return {u8"lvb", Length, LengthBasis::Viewport};
      case lvmin:      return {u8"lvmin", Length, LengthBasis::Viewport};
      case lvmax:      return {u8"lvmax", Length, LengthBasis::Viewport};
      case dvw:        return {u8"dvw", Length, LengthBasis::Viewport};
      case dvh:        return {u8"dvh", Length, LengthBasis::Viewport};
      case dvi:        return {u8"dvi", Length, LengthBasis::Viewport};
      case dvb:        return {u8"dvb", Length, LengthBasis::Viewport};
      case dvmin:      return {u8"dvmin", Length, LengthBasis::Viewport};
      case dvmax:      return {u8"dvmax", Length, LengthBasis::Viewport};

      case cqw:        return {u8"cqw", Length, LengthBasis::Container};
      case cqh:        return {u8"cqh", Length, LengthBasis::Container};
      case cqi:        return {u8"cqi", Length, LengthBasis::Container};
      case cqb:        return {u8"cqb", Length, LengthBasis::Container};
      case cqmin:      return {u8"cqmin", Length, LengthBasis::Container};
      case cqmax:      return {u8"cqmax", Length, LengthBasis::Container};

      // https://drafts.csswg.org/css-values-4/#angles
      case deg:        return {u8"deg", Category::Angle, {}, 1.0};
      case rad:        return {u8"rad", Category::Angle, {}, degreesPerRadianDouble};
      case grad:       return {u8"grad", Category::Angle, {}, degreesPerGradientDouble};
      case turn:       return {u8"turn", Category::Angle, {}, degreesPerTurnDouble};

      case s:          return {u8"s", Category::Time, {}, 1.0};
      case ms:         return {u8"ms", Category::Time, {}, secsPerMs};

      case hz:         return {u8"hz", Category::Frequency, {}, 1.0};
      case khz:        return {u8"khz", Category::Frequency, {}, hertzPerKilohertz};

      case dppx:       return {u8"dppx", Category::Resolution, {}, 1.0};
      case x:          return {u8"x", Category::Resolution, {}, dppxPerX};
      case dpi:        return {u8"dpi", Category::Resolution, {}, dppxPerDpi};
      case dpcm:       return {u8"dpcm", Category::Resolution, {}, dppxPerDpcm};

      // https://drafts.csswg.org/css-grid/#fr-unit
      case fr:         return {u8"fr", Category::Flex};

      case Unknown:    return {};
    }
    krys_unreachable();
  }

#pragma region Names

  /// @brief The unit a dimension token's unit names, ignoring ASCII case.
  /// @return `UnitType::Unknown` if it names none.
  KRYS_NODISCARD constexpr UnitType ParseUnitType(utf8_stringview name) noexcept
  {
    auto equalsIgnoringCase = [&](utf8_stringview lowercase)
    {
      if (lowercase.size() != name.size())
      {
        return false;
      }
      for (size_t i = 0; i < name.size(); ++i)
      {
        if (text::ToASCIILower(name[i]) != lowercase[i])
        {
          return false;
        }
      }
      return true;
    };

    if (equalsIgnoringCase(u8"__qem"))
    {
      return UnitType::QuirkyEm;
    }
    for (auto i = 0uz; i <= ToUnderlying(MaxEnumValue(UnitType {})); ++i)
    {
      auto unit = static_cast<UnitType>(i);
      auto info = UnitInfoFor(unit);
      // `%` is a token of its own, not a dimension's unit.
      if (!info.Name.empty() && unit != UnitType::Percentage && equalsIgnoringCase(info.Name))
      {
        return unit;
      }
    }
    return UnitType::Unknown;
  }

  /// @brief @p unit as it's serialised after a number: empty for `Unknown`, numbers and integers.
  KRYS_NODISCARD constexpr utf8_stringview UnitTypeString(UnitType unit) noexcept
  {
    return UnitInfoFor(unit).Name;
  }

#pragma endregion

#pragma region Kinds of unit

  KRYS_NODISCARD constexpr bool IsLength(UnitType unit) noexcept
  {
    return UnitInfoFor(unit).Quantity == Category::Length;
  }

  /// @brief Whether @p unit is a length relative to a font, the root's included.
  /// @see https://drafts.csswg.org/css-values-4/#font-relative-lengths
  KRYS_NODISCARD constexpr bool IsFontRelativeLength(UnitType unit) noexcept
  {
    auto basis = UnitInfoFor(unit).Basis;
    return IsLength(unit) && (basis == LengthBasis::Font || basis == LengthBasis::RootFont);
  }

  KRYS_NODISCARD constexpr bool IsRootFontRelativeLength(UnitType unit) noexcept
  {
    return IsLength(unit) && UnitInfoFor(unit).Basis == LengthBasis::RootFont;
  }

  /// @brief Whether @p unit is an absolute length, which no font affects.
  /// @see https://drafts.csswg.org/css-values-4/#absolute-lengths
  KRYS_NODISCARD constexpr bool IsFontIndependentLength(UnitType unit) noexcept
  {
    return IsLength(unit) && UnitInfoFor(unit).Basis == LengthBasis::Absolute;
  }

  /// @see https://drafts.csswg.org/css-values-4/#viewport-relative-lengths
  KRYS_NODISCARD constexpr bool IsViewportPercentageLength(UnitType unit) noexcept
  {
    return IsLength(unit) && UnitInfoFor(unit).Basis == LengthBasis::Viewport;
  }

  /// @see https://drafts.csswg.org/css-conditional-5/#container-lengths
  KRYS_NODISCARD constexpr bool IsContainerPercentageLength(UnitType unit) noexcept
  {
    return IsLength(unit) && UnitInfoFor(unit).Basis == LengthBasis::Container;
  }

  KRYS_NODISCARD constexpr UnitCategory UnitCategoryForType(UnitType unit) noexcept
  {
    auto info = UnitInfoFor(unit);
    if (!info.Quantity)
    {
      return UnitCategory::Other;
    }
    switch (*info.Quantity)
    {
      case Category::Integer:
      case Category::Number:     return UnitCategory::Number;
      case Category::Percentage: return UnitCategory::Percent;
      case Category::Angle:      return UnitCategory::Angle;
      case Category::Time:       return UnitCategory::Time;
      case Category::Frequency:  return UnitCategory::Frequency;
      case Category::Resolution: return UnitCategory::Resolution;
      case Category::Flex:       return UnitCategory::Flex;
      case Category::Length:
        switch (info.Basis)
        {
          case LengthBasis::Absolute:  return UnitCategory::AbsoluteLength;
          case LengthBasis::Font:
          case LengthBasis::RootFont:  return UnitCategory::FontRelativeLength;
          case LengthBasis::Viewport:  return UnitCategory::ViewportPercentageLength;
          case LengthBasis::Container: return UnitCategory::Other;
        }
        break;
      case Category::LengthPercentage:
      case Category::AnglePercentage:  break;
    }
    krys_unreachable();
  }

#pragma endregion

#pragma region Conversion

  /// @brief The unit that the units of @p category convert to; `UnitType::Unknown` for relative lengths,
  /// which only resolve with a context, and for `Other`.
  KRYS_NODISCARD constexpr UnitType CanonicalUnitTypeForCategory(UnitCategory category) noexcept
  {
    switch (category)
    {
      case UnitCategory::Number:                   return UnitType::Number;
      case UnitCategory::Percent:                  return UnitType::Percentage;
      case UnitCategory::AbsoluteLength:           return UnitType::px;
      case UnitCategory::Angle:                    return UnitType::deg;
      case UnitCategory::Time:                     return UnitType::s;
      case UnitCategory::Frequency:                return UnitType::hz;
      case UnitCategory::Resolution:               return UnitType::dppx;
      case UnitCategory::Flex:                     return UnitType::fr;
      case UnitCategory::FontRelativeLength:
      case UnitCategory::ViewportPercentageLength:
      case UnitCategory::Other:                    return UnitType::Unknown;
    }
    krys_unreachable();
  }

  KRYS_NODISCARD constexpr UnitType CanonicalUnitTypeForUnitType(UnitType unit) noexcept
  {
    return CanonicalUnitTypeForCategory(UnitCategoryForType(unit));
  }

  /// @brief The fixed factor from @p unit to its canonical unit, if it has one.
  KRYS_NODISCARD constexpr Maybe<double> ConversionToCanonicalUnitsScaleFactor(UnitType unit) noexcept
  {
    return UnitInfoFor(unit).ScaleToCanonical;
  }

  /// @brief Whether converting @p unit to its canonical unit needs a `ToLengthConversionData`: for every
  /// length but `px`.
  KRYS_NODISCARD constexpr bool ConversionToCanonicalUnitRequiresConversionData(UnitType unit) noexcept
  {
    return IsLength(unit) && unit != UnitType::px;
  }

  /// @brief @p value in @p from converted to @p to, through their canonical unit.
  /// @return Null unless both units measure the same quantity and have fixed factors.
  KRYS_NODISCARD constexpr Maybe<double> ConvertUnit(double value, UnitType from, UnitType to) noexcept
  {
    if (from == to)
    {
      return value;
    }
    auto fromInfo = UnitInfoFor(from);
    auto toInfo = UnitInfoFor(to);
    if (!fromInfo.Quantity || fromInfo.Quantity != toInfo.Quantity || !fromInfo.ScaleToCanonical
        || !toInfo.ScaleToCanonical)
    {
      return null;
    }
    return value * *fromInfo.ScaleToCanonical / *toInfo.ScaleToCanonical;
  }

#pragma endregion
}

#pragma once

#include "Krystal.Booey/CSS/Values/UnitCategory.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css
{
  // We always assume 96 CSS pixels in a CSS inch. This is the cold hard truth of the Web.
  // At high DPI, we may scale a CSS pixel, but the ratio of the CSS pixel to the so-called
  // "absolute" CSS length units like inch and pt is always fixed and never changes.
  constexpr double pxPerInch = 96;

  constexpr double ptsPerInch = 72;
  constexpr double picasPerInch = 6;
  constexpr double mmPerInch = 25.4;
  constexpr double cmPerInch = 2.54;
  constexpr double qPerInch = 25.4 * 4.0;

  constexpr double pxPerCm = pxPerInch / cmPerInch;
  constexpr double pxPerMm = pxPerInch / mmPerInch;
  constexpr double pxPerQ = pxPerInch / qPerInch;
  constexpr double pxPerPt = pxPerInch / ptsPerInch;
  constexpr double pxPerPc = pxPerInch / picasPerInch;
  constexpr double dppxPerX = 1.0;
  constexpr double dppxPerDpi = 1.0 / pxPerInch;
  constexpr double dppxPerDpcm = cmPerInch / pxPerInch;
  constexpr double secsPerMs = 1.0 / 1000.0;
  constexpr double hertzPerKilohertz = 1000.0;

  KRYS_NODISCARD constexpr UnitCategory UnitCategoryForType(UnitType type) noexcept
  {
    switch (type)
    {
      case UnitType::Number:
      case UnitType::Integer:
      {
        return UnitCategory::Number;
      }
      case UnitType::Percentage:
      {
        return UnitCategory::Percent;
      }
      // https://drafts.csswg.org/css-values-4/#absolute-lengths
      case UnitType::px:
      case UnitType::cm:
      case UnitType::mm:
      case UnitType::in:
      case UnitType::pt:
      case UnitType::pc:
      case UnitType::Q:
      {
        return UnitCategory::AbsoluteLength;
      }
      // https://drafts.csswg.org/css-values-4/#font-relative-lengths
      case UnitType::em:
      case UnitType::ex:
      case UnitType::cap:
      case UnitType::ch:
      case UnitType::ic:
      case UnitType::lh:
      case UnitType::rcap:
      case UnitType::rch:
      case UnitType::rem:
      case UnitType::rex:
      case UnitType::ric:
      case UnitType::rlh:
      {
        return UnitCategory::FontRelativeLength;
      }
      // https://drafts.csswg.org/css-values-4/#viewport-relative-lengths
      case UnitType::vw:
      case UnitType::svw:
      case UnitType::lvw:
      case UnitType::dvw:
      case UnitType::vh:
      case UnitType::svh:
      case UnitType::lvh:
      case UnitType::dvh:
      case UnitType::vi:
      case UnitType::svi:
      case UnitType::lvi:
      case UnitType::dvi:
      case UnitType::vb:
      case UnitType::svb:
      case UnitType::lvb:
      case UnitType::dvb:
      case UnitType::vmin:
      case UnitType::lvmin:
      case UnitType::svmin:
      case UnitType::dvmin:
      case UnitType::vmax:
      case UnitType::svmax:
      case UnitType::lvmax:
      case UnitType::dvmax:
      {
        return UnitCategory::ViewportPercentageLength;
      }
      // https://drafts.csswg.org/css-values-4/#time
      case UnitType::ms:
      case UnitType::s:
      {
        return UnitCategory::Time;
      }
      // https://drafts.csswg.org/css-values-4/#angles
      case UnitType::deg:
      case UnitType::rad:
      case UnitType::grad:
      case UnitType::turn:
      {
        return UnitCategory::Angle;
      }
      // https://drafts.csswg.org/css-values-4/#frequency
      case UnitType::hz:
      case UnitType::khz:
      {
        return UnitCategory::Frequency;
      }
      // https://drafts.csswg.org/css-values-4/#resolution
      case UnitType::dppx:
      case UnitType::x:
      case UnitType::dpi:
      case UnitType::dpcm:
      {
        return UnitCategory::Resolution;
      }
      case UnitType::fr:
      {
        return UnitCategory::Flex;
      }
      case UnitType::cqw:
      case UnitType::cqh:
      case UnitType::cqi:
      case UnitType::cqb:
      case UnitType::cqmin:
      case UnitType::cqmax:
      case UnitType::Attr:
      case UnitType::Calc:
      case UnitType::CalcPercentageWithAngle:
      case UnitType::CalcPercentageWithLength:
      case UnitType::Dimension:
      case UnitType::FontFamily:
      case UnitType::Ident:
      case UnitType::PropertyId:
      case UnitType::String:
      case UnitType::Unknown:
      case UnitType::ValueId:
      case UnitType::CustomIdent:
      {
        return UnitCategory::Other;
      }
    }

    krys_unreachable();
  }

  KRYS_NODISCARD constexpr UnitType CanonicalUnitTypeForCategory(UnitCategory category) noexcept
  {
    switch (category)
    {
      case UnitCategory::Number:
      {
        return UnitType::Number;
      }
      case UnitCategory::AbsoluteLength:
      {
        return UnitType::px;
      }
      case UnitCategory::Percent:
      {
        return UnitType::Percentage;
      }
      case UnitCategory::Time:
      {
        return UnitType::s;
      }
      case UnitCategory::Angle:
      {
        return UnitType::deg;
      }
      case UnitCategory::Frequency:
      {
        return UnitType::hz;
      }
      case UnitCategory::Resolution:
      {
        return UnitType::dppx;
      }
      case UnitCategory::Flex:
      {
        return UnitType::fr;
      }
      case UnitCategory::FontRelativeLength:
      case UnitCategory::ViewportPercentageLength:
      case UnitCategory::Other:
      {
        return UnitType::Unknown;
      }
    }

    krys_unreachable();
  }

  KRYS_NODISCARD constexpr UnitType CanonicalUnitTypeForUnitType(UnitType unitType) noexcept
  {
    return CanonicalUnitTypeForCategory(UnitCategory(unitType));
  }
}

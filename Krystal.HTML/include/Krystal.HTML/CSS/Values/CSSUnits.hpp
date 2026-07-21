#pragma once

#include "Krystal.HTML/CSS/Values/Enums/CSSUnitCategory.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSUnitType.hpp"
#include <cassert>

namespace Krys::HTML
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

  KRYS_NODISCARD constexpr CSSUnitCategory UnitCategory(CSSUnitType type) noexcept
  {
    switch (type)
    {
      case CSSUnitType::Number:
      case CSSUnitType::Integer:
      {
        return CSSUnitCategory::Number;
      }
      case CSSUnitType::Percentage:
      {
        return CSSUnitCategory::Percent;
      }
      // https://drafts.csswg.org/css-values-4/#absolute-lengths
      case CSSUnitType::px:
      case CSSUnitType::cm:
      case CSSUnitType::mm:
      case CSSUnitType::in:
      case CSSUnitType::pt:
      case CSSUnitType::pc:
      case CSSUnitType::Q:
      {
        return CSSUnitCategory::AbsoluteLength;
      }
      // https://drafts.csswg.org/css-values-4/#font-relative-lengths
      case CSSUnitType::em:
      case CSSUnitType::ex:
      case CSSUnitType::cap:
      case CSSUnitType::ch:
      case CSSUnitType::ic:
      case CSSUnitType::lh:
      case CSSUnitType::rcap:
      case CSSUnitType::rch:
      case CSSUnitType::rem:
      case CSSUnitType::rex:
      case CSSUnitType::ric:
      case CSSUnitType::rlh:
      {
        return CSSUnitCategory::FontRelativeLength;
      }
      // https://drafts.csswg.org/css-values-4/#viewport-relative-lengths
      case CSSUnitType::vw:
      case CSSUnitType::svw:
      case CSSUnitType::lvw:
      case CSSUnitType::dvw:
      case CSSUnitType::vh:
      case CSSUnitType::svh:
      case CSSUnitType::lvh:
      case CSSUnitType::dvh:
      case CSSUnitType::vi:
      case CSSUnitType::svi:
      case CSSUnitType::lvi:
      case CSSUnitType::dvi:
      case CSSUnitType::vb:
      case CSSUnitType::svb:
      case CSSUnitType::lvb:
      case CSSUnitType::dvb:
      case CSSUnitType::vmin:
      case CSSUnitType::lvmin:
      case CSSUnitType::svmin:
      case CSSUnitType::dvmin:
      case CSSUnitType::vmax:
      case CSSUnitType::svmax:
      case CSSUnitType::lvmax:
      case CSSUnitType::dvmax:
      {
        return CSSUnitCategory::ViewportPercentageLength;
      }
      // https://drafts.csswg.org/css-values-4/#time
      case CSSUnitType::ms:
      case CSSUnitType::s:
      {
        return CSSUnitCategory::Time;
      }
      // https://drafts.csswg.org/css-values-4/#angles
      case CSSUnitType::deg:
      case CSSUnitType::rad:
      case CSSUnitType::grad:
      case CSSUnitType::turn:
      {
        return CSSUnitCategory::Angle;
      }
      // https://drafts.csswg.org/css-values-4/#frequency
      case CSSUnitType::hz:
      case CSSUnitType::khz:
      {
        return CSSUnitCategory::Frequency;
      }
      // https://drafts.csswg.org/css-values-4/#resolution
      case CSSUnitType::dppx:
      case CSSUnitType::x:
      case CSSUnitType::dpi:
      case CSSUnitType::dpcm:
      {
        return CSSUnitCategory::Resolution;
      }
      case CSSUnitType::fr:
      {
        return CSSUnitCategory::Flex;
      }
      case CSSUnitType::cqw:
      case CSSUnitType::cqh:
      case CSSUnitType::cqi:
      case CSSUnitType::cqb:
      case CSSUnitType::cqmin:
      case CSSUnitType::cqmax:
      case CSSUnitType::Attr:
      case CSSUnitType::Calc:
      case CSSUnitType::CalcPercentageWithAngle:
      case CSSUnitType::CalcPercentageWithLength:
      case CSSUnitType::Dimension:
      case CSSUnitType::FontFamily:
      case CSSUnitType::Ident:
      case CSSUnitType::PropertyId:
      case CSSUnitType::String:
      case CSSUnitType::Unknown:
      case CSSUnitType::ValueId:
      case CSSUnitType::CustomIdent:
      {
        return CSSUnitCategory::Other;
      }
    }

    assert(false);
    return CSSUnitCategory::Other;
  }

  KRYS_NODISCARD constexpr CSSUnitType CanonicalUnitTypeForCategory(CSSUnitCategory category) noexcept
  {
    switch (category)
    {
      case CSSUnitCategory::Number:
      {
        return CSSUnitType::Number;
      }
      case CSSUnitCategory::AbsoluteLength:
      {
        return CSSUnitType::px;
      }
      case CSSUnitCategory::Percent:
      {
        return CSSUnitType::Percentage;
      }
      case CSSUnitCategory::Time:
      {
        return CSSUnitType::s;
      }
      case CSSUnitCategory::Angle:
      {
        return CSSUnitType::deg;
      }
      case CSSUnitCategory::Frequency:
      {
        return CSSUnitType::hz;
      }
      case CSSUnitCategory::Resolution:
      {
        return CSSUnitType::dppx;
      }
      case CSSUnitCategory::Flex:
      {
        return CSSUnitType::fr;
      }
      case CSSUnitCategory::FontRelativeLength:
      case CSSUnitCategory::ViewportPercentageLength:
      case CSSUnitCategory::Other:
      {
        return CSSUnitType::Unknown;
      }
    }
    assert(false);
    return CSSUnitType::Unknown;
  }

  KRYS_NODISCARD constexpr CSSUnitType CanonicalUnitTypeForUnitType(CSSUnitType unitType) noexcept
  {
    return CanonicalUnitTypeForCategory(UnitCategory(unitType));
  }
}

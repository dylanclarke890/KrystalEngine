#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericUnits.hpp"

namespace Krys::HTML
{
  utf8_stringview UnitString(AngleUnit angleUnit) noexcept
  {
    switch (angleUnit)
    {
      case AngleUnit::Deg:  return u8"deg";
      case AngleUnit::Rad:  return u8"rad";
      case AngleUnit::Grad: return u8"grad";
      case AngleUnit::Turn: return u8"turn";
    }

    std::unreachable();
  }

  utf8_stringview UnitString(LengthUnit lengthUnit) noexcept
  {
    switch (lengthUnit)
    {
      case LengthUnit::Px:    return u8"px";
      case LengthUnit::Cm:    return u8"cm";
      case LengthUnit::Mm:    return u8"mm";
      case LengthUnit::Q:     return u8"Q";
      case LengthUnit::In:    return u8"in";
      case LengthUnit::Pt:    return u8"pt";
      case LengthUnit::Pc:    return u8"pc";
      case LengthUnit::Em:    return u8"em";
      case LengthUnit::Ex:    return u8"ex";
      case LengthUnit::Lh:    return u8"lh";
      case LengthUnit::Cap:   return u8"cap";
      case LengthUnit::Ch:    return u8"ch";
      case LengthUnit::Ic:    return u8"ic";
      case LengthUnit::Rcap:  return u8"rcap";
      case LengthUnit::Rch:   return u8"rch";
      case LengthUnit::Rem:   return u8"rem";
      case LengthUnit::Rex:   return u8"rex";
      case LengthUnit::Ric:   return u8"ric";
      case LengthUnit::Rlh:   return u8"rlh";
      case LengthUnit::Vw:    return u8"vw";
      case LengthUnit::Vh:    return u8"vh";
      case LengthUnit::Vmin:  return u8"vmin";
      case LengthUnit::Vmax:  return u8"vmax";
      case LengthUnit::Vb:    return u8"vb";
      case LengthUnit::Vi:    return u8"vi";
      case LengthUnit::Svw:   return u8"svw";
      case LengthUnit::Svh:   return u8"svh";
      case LengthUnit::Svmin: return u8"svmin";
      case LengthUnit::Svmax: return u8"svmax";
      case LengthUnit::Svb:   return u8"svb";
      case LengthUnit::Svi:   return u8"svi";
      case LengthUnit::Lvw:   return u8"lvw";
      case LengthUnit::Lvh:   return u8"lvh";
      case LengthUnit::Lvmin: return u8"lvmin";
      case LengthUnit::Lvmax: return u8"lvmax";
      case LengthUnit::Lvb:   return u8"lvb";
      case LengthUnit::Lvi:   return u8"lvi";
      case LengthUnit::Dvw:   return u8"dvw";
      case LengthUnit::Dvh:   return u8"dvh";
      case LengthUnit::Dvmin: return u8"dvmin";
      case LengthUnit::Dvmax: return u8"dvmax";
      case LengthUnit::Dvb:   return u8"dvb";
      case LengthUnit::Dvi:   return u8"dvi";
      case LengthUnit::Cqw:   return u8"cqw";
      case LengthUnit::Cqh:   return u8"cqh";
      case LengthUnit::Cqi:   return u8"cqi";
      case LengthUnit::Cqb:   return u8"cqb";
      case LengthUnit::Cqmin: return u8"cqmin";
      case LengthUnit::Cqmax: return u8"cqmax";
    }

    std::unreachable();
  }

  utf8_stringview UnitString(TimeUnit timeUnit) noexcept
  {
    switch (timeUnit)
    {
      case TimeUnit::S:  return u8"s";
      case TimeUnit::Ms: return u8"ms";
    }

    std::unreachable();
  }

  utf8_stringview UnitString(FrequencyUnit frequencyUnit) noexcept
  {
    switch (frequencyUnit)
    {
      case FrequencyUnit::Hz:  return u8"hz";
      case FrequencyUnit::Khz: return u8"khz";
    }

    std::unreachable();
  }

  utf8_stringview UnitString(ResolutionUnit resolutionUnit) noexcept
  {
    switch (resolutionUnit)
    {
      case ResolutionUnit::Dppx: return u8"dppx";
      case ResolutionUnit::X:    return u8"x";
      case ResolutionUnit::Dpi:  return u8"dpi";
      case ResolutionUnit::Dpcm: return u8"dpcm";
    }

    std::unreachable();
  }

  // Ensure the angle units in `AngleUnit` and `AnglePercentageUnit` are all equal.
  static_assert(ToUnderlying(AngleUnit::Deg) == ToUnderlying(AnglePercentageUnit::Deg));
  static_assert(ToUnderlying(AngleUnit::Rad) == ToUnderlying(AnglePercentageUnit::Rad));
  static_assert(ToUnderlying(AngleUnit::Grad) == ToUnderlying(AnglePercentageUnit::Grad));
  static_assert(ToUnderlying(AngleUnit::Turn) == ToUnderlying(AnglePercentageUnit::Turn));

  // Ensure the length units in `LengthUnit` and `LengthPercentageUnit` are all equal.
  static_assert(ToUnderlying(LengthUnit::Px) == ToUnderlying(LengthPercentageUnit::Px));
  static_assert(ToUnderlying(LengthUnit::Cm) == ToUnderlying(LengthPercentageUnit::Cm));
  static_assert(ToUnderlying(LengthUnit::Mm) == ToUnderlying(LengthPercentageUnit::Mm));
  static_assert(ToUnderlying(LengthUnit::Q) == ToUnderlying(LengthPercentageUnit::Q));
  static_assert(ToUnderlying(LengthUnit::In) == ToUnderlying(LengthPercentageUnit::In));
  static_assert(ToUnderlying(LengthUnit::Pt) == ToUnderlying(LengthPercentageUnit::Pt));
  static_assert(ToUnderlying(LengthUnit::Pc) == ToUnderlying(LengthPercentageUnit::Pc));
  static_assert(ToUnderlying(LengthUnit::Em) == ToUnderlying(LengthPercentageUnit::Em));
  static_assert(ToUnderlying(LengthUnit::Ex) == ToUnderlying(LengthPercentageUnit::Ex));
  static_assert(ToUnderlying(LengthUnit::Lh) == ToUnderlying(LengthPercentageUnit::Lh));
  static_assert(ToUnderlying(LengthUnit::Cap) == ToUnderlying(LengthPercentageUnit::Cap));
  static_assert(ToUnderlying(LengthUnit::Ch) == ToUnderlying(LengthPercentageUnit::Ch));
  static_assert(ToUnderlying(LengthUnit::Ic) == ToUnderlying(LengthPercentageUnit::Ic));
  static_assert(ToUnderlying(LengthUnit::Rcap) == ToUnderlying(LengthPercentageUnit::Rcap));
  static_assert(ToUnderlying(LengthUnit::Rch) == ToUnderlying(LengthPercentageUnit::Rch));
  static_assert(ToUnderlying(LengthUnit::Rem) == ToUnderlying(LengthPercentageUnit::Rem));
  static_assert(ToUnderlying(LengthUnit::Rex) == ToUnderlying(LengthPercentageUnit::Rex));
  static_assert(ToUnderlying(LengthUnit::Ric) == ToUnderlying(LengthPercentageUnit::Ric));
  static_assert(ToUnderlying(LengthUnit::Rlh) == ToUnderlying(LengthPercentageUnit::Rlh));
  static_assert(ToUnderlying(LengthUnit::Vw) == ToUnderlying(LengthPercentageUnit::Vw));
  static_assert(ToUnderlying(LengthUnit::Vh) == ToUnderlying(LengthPercentageUnit::Vh));
  static_assert(ToUnderlying(LengthUnit::Vmin) == ToUnderlying(LengthPercentageUnit::Vmin));
  static_assert(ToUnderlying(LengthUnit::Vmax) == ToUnderlying(LengthPercentageUnit::Vmax));
  static_assert(ToUnderlying(LengthUnit::Vb) == ToUnderlying(LengthPercentageUnit::Vb));
  static_assert(ToUnderlying(LengthUnit::Vi) == ToUnderlying(LengthPercentageUnit::Vi));
  static_assert(ToUnderlying(LengthUnit::Svw) == ToUnderlying(LengthPercentageUnit::Svw));
  static_assert(ToUnderlying(LengthUnit::Svh) == ToUnderlying(LengthPercentageUnit::Svh));
  static_assert(ToUnderlying(LengthUnit::Svmin) == ToUnderlying(LengthPercentageUnit::Svmin));
  static_assert(ToUnderlying(LengthUnit::Svmax) == ToUnderlying(LengthPercentageUnit::Svmax));
  static_assert(ToUnderlying(LengthUnit::Svb) == ToUnderlying(LengthPercentageUnit::Svb));
  static_assert(ToUnderlying(LengthUnit::Svi) == ToUnderlying(LengthPercentageUnit::Svi));
  static_assert(ToUnderlying(LengthUnit::Lvw) == ToUnderlying(LengthPercentageUnit::Lvw));
  static_assert(ToUnderlying(LengthUnit::Lvh) == ToUnderlying(LengthPercentageUnit::Lvh));
  static_assert(ToUnderlying(LengthUnit::Lvmin) == ToUnderlying(LengthPercentageUnit::Lvmin));
  static_assert(ToUnderlying(LengthUnit::Lvmax) == ToUnderlying(LengthPercentageUnit::Lvmax));
  static_assert(ToUnderlying(LengthUnit::Lvb) == ToUnderlying(LengthPercentageUnit::Lvb));
  static_assert(ToUnderlying(LengthUnit::Lvi) == ToUnderlying(LengthPercentageUnit::Lvi));
  static_assert(ToUnderlying(LengthUnit::Dvw) == ToUnderlying(LengthPercentageUnit::Dvw));
  static_assert(ToUnderlying(LengthUnit::Dvh) == ToUnderlying(LengthPercentageUnit::Dvh));
  static_assert(ToUnderlying(LengthUnit::Dvmin) == ToUnderlying(LengthPercentageUnit::Dvmin));
  static_assert(ToUnderlying(LengthUnit::Dvmax) == ToUnderlying(LengthPercentageUnit::Dvmax));
  static_assert(ToUnderlying(LengthUnit::Dvb) == ToUnderlying(LengthPercentageUnit::Dvb));
  static_assert(ToUnderlying(LengthUnit::Dvi) == ToUnderlying(LengthPercentageUnit::Dvi));
  static_assert(ToUnderlying(LengthUnit::Cqw) == ToUnderlying(LengthPercentageUnit::Cqw));
  static_assert(ToUnderlying(LengthUnit::Cqh) == ToUnderlying(LengthPercentageUnit::Cqh));
  static_assert(ToUnderlying(LengthUnit::Cqi) == ToUnderlying(LengthPercentageUnit::Cqi));
  static_assert(ToUnderlying(LengthUnit::Cqb) == ToUnderlying(LengthPercentageUnit::Cqb));
  static_assert(ToUnderlying(LengthUnit::Cqmin) == ToUnderlying(LengthPercentageUnit::Cqmin));
  static_assert(ToUnderlying(LengthUnit::Cqmax) == ToUnderlying(LengthPercentageUnit::Cqmax));
}
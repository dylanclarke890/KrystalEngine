#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::css
{
  enum class UnitType : uint8
  {
    Unknown,
    Number,
    Integer,
    Percentage,
    em,
    ex,
    px,
    cm,
    mm,
    in,
    pt,
    pc,
    deg,
    rad,
    grad,
    ms,
    s,
    hz,
    khz,
    Dimension,
    String,
    Ident,
    Attr,

    vw,
    vh,
    vmin,
    vmax,
    vb,
    vi,
    svw,
    svh,
    svmin,
    svmax,
    svb,
    svi,
    lvw,
    lvh,
    lvmin,
    lvmax,
    lvb,
    lvi,
    dvw,
    dvh,
    dvmin,
    dvmax,
    dvb,
    dvi,

    FirstViewportCSSUnitType = vw,
    LastViewportCSSUnitType = dvi,

    cqw,
    cqh,
    cqi,
    cqb,
    cqmin,
    cqmax,

    dppx,
    x,
    dpi,
    dpcm,
    fr,
    Q,
    lh,
    rlh,

    CustomIdent,

    turn,
    rem,
    rex,
    cap,
    rcap,
    ch,
    rch,
    ic,
    ric,

    Calc,
    CalcPercentageWithAngle,
    CalcPercentageWithLength,

    FontFamily,

    PropertyId,
    ValueId,

    QuirkyEm,
  };

  KRYS_NODISCARD constexpr UnitType ParseUnitType(utf8_stringview str) noexcept
  {
    // TODO: Implement parsing logic for UnitType from string representation.
    return UnitType::Unknown;
  }
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::UnitType, 77uz);

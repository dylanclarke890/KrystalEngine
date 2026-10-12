#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
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

  /// @brief Maps a dimension token's unit to its UnitType, ignoring ASCII case.
  /// @return `UnitType::Unknown` if the unit isn't recognised.
  KRYS_NODISCARD constexpr UnitType ParseUnitType(utf8_stringview str) noexcept
  {
    struct Entry
    {
      utf8_stringview Name;
      UnitType Unit;
    };

    constexpr Entry units[] = {
      {u8"em", UnitType::em},       {u8"ex", UnitType::ex},       {u8"px", UnitType::px},
      {u8"cm", UnitType::cm},       {u8"mm", UnitType::mm},       {u8"in", UnitType::in},
      {u8"pt", UnitType::pt},       {u8"pc", UnitType::pc},       {u8"deg", UnitType::deg},
      {u8"rad", UnitType::rad},     {u8"grad", UnitType::grad},   {u8"ms", UnitType::ms},
      {u8"s", UnitType::s},         {u8"hz", UnitType::hz},       {u8"khz", UnitType::khz},
      {u8"vw", UnitType::vw},       {u8"vh", UnitType::vh},       {u8"vmin", UnitType::vmin},
      {u8"vmax", UnitType::vmax},   {u8"vb", UnitType::vb},       {u8"vi", UnitType::vi},
      {u8"svw", UnitType::svw},     {u8"svh", UnitType::svh},     {u8"svmin", UnitType::svmin},
      {u8"svmax", UnitType::svmax}, {u8"svb", UnitType::svb},     {u8"svi", UnitType::svi},
      {u8"lvw", UnitType::lvw},     {u8"lvh", UnitType::lvh},     {u8"lvmin", UnitType::lvmin},
      {u8"lvmax", UnitType::lvmax}, {u8"lvb", UnitType::lvb},     {u8"lvi", UnitType::lvi},
      {u8"dvw", UnitType::dvw},     {u8"dvh", UnitType::dvh},     {u8"dvmin", UnitType::dvmin},
      {u8"dvmax", UnitType::dvmax}, {u8"dvb", UnitType::dvb},     {u8"dvi", UnitType::dvi},
      {u8"cqw", UnitType::cqw},     {u8"cqh", UnitType::cqh},     {u8"cqi", UnitType::cqi},
      {u8"cqb", UnitType::cqb},     {u8"cqmin", UnitType::cqmin}, {u8"cqmax", UnitType::cqmax},
      {u8"dppx", UnitType::dppx},   {u8"x", UnitType::x},         {u8"dpi", UnitType::dpi},
      {u8"dpcm", UnitType::dpcm},   {u8"fr", UnitType::fr},       {u8"q", UnitType::Q},
      {u8"lh", UnitType::lh},       {u8"rlh", UnitType::rlh},     {u8"turn", UnitType::turn},
      {u8"rem", UnitType::rem},     {u8"rex", UnitType::rex},     {u8"cap", UnitType::cap},
      {u8"rcap", UnitType::rcap},   {u8"ch", UnitType::ch},       {u8"rch", UnitType::rch},
      {u8"ic", UnitType::ic},       {u8"ric", UnitType::ric},     {u8"__qem", UnitType::QuirkyEm},
    };

    for (const auto &entry : units)
    {
      if (entry.Name.size() != str.size())
      {
        continue;
      }

      bool matches = true;
      for (size_t i = 0; i < str.size(); ++i)
      {
        if (text::ToASCIILower(str[i]) != entry.Name[i])
        {
          matches = false;
          break;
        }
      }

      if (matches)
      {
        return entry.Unit;
      }
    }

    return UnitType::Unknown;
  }
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::UnitType, 78uz);

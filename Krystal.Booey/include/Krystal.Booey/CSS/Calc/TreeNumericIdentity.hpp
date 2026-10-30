#pragma once

#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css::calc
{
  // `NumericIdentity` is used to assign a densely packed unique identifier for each possible numeric type
  // allowed in a `calc()` expression. This allows the construction of fixed size (specifically sized to
  // `NumberOfNumericIdentityTypes`) lookup tables needed in expression simplification.

  enum class NumericIdentity : uint8
  {
    Number,
    Percentage,

    // Canonical dimension units
    PX,
    DEG,
    S,
    HZ,
    DPPX,
    FR,

    // Non-canonical dimension units
    CM,
    MM,
    Q,
    INCH, // NOTE: using "IN" here breaks under MSVC.
    PT,
    PC,
    EM,
    EX,
    LH,
    CAP,
    CH,
    IC,
    RCAP,
    RCH,
    REM,
    REX,
    RIC,
    RLH,
    VW,
    VH,
    VMIN,
    VMAX,
    VB,
    VI,
    SVW,
    SVH,
    SVMIN,
    SVMAX,
    SVB,
    SVI,
    LVW,
    LVH,
    LVMIN,
    LVMAX,
    LVB,
    LVI,
    DVW,
    DVH,
    DVMIN,
    DVMAX,
    DVB,
    DVI,
    CQW,
    CQH,
    CQI,
    CQB,
    CQMIN,
    CQMAX,
    RAD,
    GRAD,
    TURN,
    MS,
    KHZ,
    X,
    DPI,
    DPCM
  };

  constexpr uint8 NumberOfNumericIdentityTypes = static_cast<uint8>(NumericIdentity::DPCM) + 1;

  KRYS_NODISCARD constexpr NumericIdentity ToNumericIdentity(const Number &) noexcept
  {
    return NumericIdentity::Number;
  }

  KRYS_NODISCARD constexpr NumericIdentity ToNumericIdentity(const Percentage &) noexcept
  {
    return NumericIdentity::Percentage;
  }

  KRYS_NODISCARD constexpr NumericIdentity ToNumericIdentity(const CanonicalDimension &dimension) noexcept
  {
    switch (dimension.Dimension)
    {
      case Dimension::Length:     return NumericIdentity::PX;
      case Dimension::Angle:      return NumericIdentity::DEG;
      case Dimension::Time:       return NumericIdentity::S;
      case Dimension::Frequency:  return NumericIdentity::HZ;
      case Dimension::Resolution: return NumericIdentity::DPPX;
      case Dimension::Flex:       return NumericIdentity::FR;
    }

    krys_unreachable();
  }

  KRYS_NODISCARD constexpr NumericIdentity ToNumericIdentity(const NonCanonicalDimension &dimension) noexcept
  {
    switch (dimension.Unit)
    {
      case UnitType::cm:                       return NumericIdentity::CM;
      case UnitType::mm:                       return NumericIdentity::MM;
      case UnitType::Q:                        return NumericIdentity::Q;
      case UnitType::in:                       return NumericIdentity::INCH;
      case UnitType::pt:                       return NumericIdentity::PT;
      case UnitType::pc:                       return NumericIdentity::PC;
      case UnitType::em:                       return NumericIdentity::EM;
      case UnitType::ex:                       return NumericIdentity::EX;
      case UnitType::lh:                       return NumericIdentity::LH;
      case UnitType::cap:                      return NumericIdentity::CAP;
      case UnitType::ch:                       return NumericIdentity::CH;
      case UnitType::ic:                       return NumericIdentity::IC;
      case UnitType::rcap:                     return NumericIdentity::RCAP;
      case UnitType::rch:                      return NumericIdentity::RCH;
      case UnitType::rem:                      return NumericIdentity::REM;
      case UnitType::rex:                      return NumericIdentity::REX;
      case UnitType::ric:                      return NumericIdentity::RIC;
      case UnitType::rlh:                      return NumericIdentity::RLH;
      case UnitType::vw:                       return NumericIdentity::VW;
      case UnitType::vh:                       return NumericIdentity::VH;
      case UnitType::vmin:                     return NumericIdentity::VMIN;
      case UnitType::vmax:                     return NumericIdentity::VMAX;
      case UnitType::vb:                       return NumericIdentity::VB;
      case UnitType::vi:                       return NumericIdentity::VI;
      case UnitType::svw:                      return NumericIdentity::SVW;
      case UnitType::svh:                      return NumericIdentity::SVH;
      case UnitType::svmin:                    return NumericIdentity::SVMIN;
      case UnitType::svmax:                    return NumericIdentity::SVMAX;
      case UnitType::svb:                      return NumericIdentity::SVB;
      case UnitType::svi:                      return NumericIdentity::SVI;
      case UnitType::lvw:                      return NumericIdentity::LVW;
      case UnitType::lvh:                      return NumericIdentity::LVH;
      case UnitType::lvmin:                    return NumericIdentity::LVMIN;
      case UnitType::lvmax:                    return NumericIdentity::LVMAX;
      case UnitType::lvb:                      return NumericIdentity::LVB;
      case UnitType::lvi:                      return NumericIdentity::LVI;
      case UnitType::dvw:                      return NumericIdentity::DVW;
      case UnitType::dvh:                      return NumericIdentity::DVH;
      case UnitType::dvmin:                    return NumericIdentity::DVMIN;
      case UnitType::dvmax:                    return NumericIdentity::DVMAX;
      case UnitType::dvb:                      return NumericIdentity::DVB;
      case UnitType::dvi:                      return NumericIdentity::DVI;
      case UnitType::cqw:                      return NumericIdentity::CQW;
      case UnitType::cqh:                      return NumericIdentity::CQH;
      case UnitType::cqi:                      return NumericIdentity::CQI;
      case UnitType::cqb:                      return NumericIdentity::CQB;
      case UnitType::cqmin:                    return NumericIdentity::CQMIN;
      case UnitType::cqmax:                    return NumericIdentity::CQMAX;
      case UnitType::rad:                      return NumericIdentity::RAD;
      case UnitType::grad:                     return NumericIdentity::GRAD;
      case UnitType::turn:                     return NumericIdentity::TURN;
      case UnitType::ms:                       return NumericIdentity::MS;
      case UnitType::khz:                      return NumericIdentity::KHZ;
      case UnitType::x:                        return NumericIdentity::X;
      case UnitType::dpi:                      return NumericIdentity::DPI;
      case UnitType::dpcm:                     return NumericIdentity::DPCM;

      // Invalid types for NonCanonicalDimension.
      case UnitType::Number:
      case UnitType::Integer:
      case UnitType::Percentage:
      case UnitType::px:
      case UnitType::deg:
      case UnitType::s:
      case UnitType::hz:
      case UnitType::dppx:
      case UnitType::fr:
      case UnitType::Attr:
      case UnitType::Calc:
      case UnitType::CalcPercentageWithAngle:
      case UnitType::CalcPercentageWithLength:
      case UnitType::Dimension:
      case UnitType::FontFamily:
      case UnitType::Ident:
      case UnitType::PropertyId:
      case UnitType::QuirkyEm:
      case UnitType::String:
      case UnitType::Unknown:
      case UnitType::ValueId:
      case UnitType::CustomIdent:              break;
    }

    krys_unreachable();
  }

  KRYS_NODISCARD constexpr bool IsLength(NumericIdentity id) noexcept
  {
    switch (id)
    {
      case NumericIdentity::PX:
      case NumericIdentity::CM:
      case NumericIdentity::MM:
      case NumericIdentity::Q:
      case NumericIdentity::INCH:
      case NumericIdentity::PT:
      case NumericIdentity::PC:
      case NumericIdentity::EM:
      case NumericIdentity::EX:
      case NumericIdentity::LH:
      case NumericIdentity::CAP:
      case NumericIdentity::CH:
      case NumericIdentity::IC:
      case NumericIdentity::RCAP:
      case NumericIdentity::RCH:
      case NumericIdentity::REM:
      case NumericIdentity::REX:
      case NumericIdentity::RIC:
      case NumericIdentity::RLH:
      case NumericIdentity::VW:
      case NumericIdentity::VH:
      case NumericIdentity::VMIN:
      case NumericIdentity::VMAX:
      case NumericIdentity::VB:
      case NumericIdentity::VI:
      case NumericIdentity::SVW:
      case NumericIdentity::SVH:
      case NumericIdentity::SVMIN:
      case NumericIdentity::SVMAX:
      case NumericIdentity::SVB:
      case NumericIdentity::SVI:
      case NumericIdentity::LVW:
      case NumericIdentity::LVH:
      case NumericIdentity::LVMIN:
      case NumericIdentity::LVMAX:
      case NumericIdentity::LVB:
      case NumericIdentity::LVI:
      case NumericIdentity::DVW:
      case NumericIdentity::DVH:
      case NumericIdentity::DVMIN:
      case NumericIdentity::DVMAX:
      case NumericIdentity::DVB:
      case NumericIdentity::DVI:
      case NumericIdentity::CQW:
      case NumericIdentity::CQH:
      case NumericIdentity::CQI:
      case NumericIdentity::CQB:
      case NumericIdentity::CQMIN:
      case NumericIdentity::CQMAX:      return true;

      case NumericIdentity::Number:
      case NumericIdentity::Percentage:
      case NumericIdentity::DEG:
      case NumericIdentity::S:
      case NumericIdentity::HZ:
      case NumericIdentity::DPPX:
      case NumericIdentity::FR:
      case NumericIdentity::RAD:
      case NumericIdentity::GRAD:
      case NumericIdentity::TURN:
      case NumericIdentity::MS:
      case NumericIdentity::KHZ:
      case NumericIdentity::X:
      case NumericIdentity::DPI:
      case NumericIdentity::DPCM:       break;
    }

    return false;
  }
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::calc::NumericIdentity,
                                   krys::boo::css::calc::NumberOfNumericIdentityTypes);
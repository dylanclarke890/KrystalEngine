#pragma once

#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  KRYS_NODISCARD constexpr bool IsCalcFunction(ValueId functionId) noexcept
  {
    switch (functionId)
    {
      case ValueId::Calc:
      case ValueId::WebkitCalc:
      case ValueId::Min:
      case ValueId::Max:
      case ValueId::Clamp:
      case ValueId::Pow:
      case ValueId::Sqrt:
      case ValueId::Hypot:
      case ValueId::Sin:
      case ValueId::Cos:
      case ValueId::Tan:
      case ValueId::Exp:
      case ValueId::Log:
      case ValueId::Asin:
      case ValueId::Acos:
      case ValueId::Atan:
      case ValueId::Atan2:
      case ValueId::Abs:
      case ValueId::Sign:
      case ValueId::Round:
      case ValueId::Mod:
      case ValueId::Rem:
      case ValueId::Progress:
      case ValueId::Random:
      case ValueId::SiblingCount:
      case ValueId::SiblingIndex:
      case ValueId::Anchor:
      case ValueId::AnchorSize:
      {
        return true;
      }
      default:
      {
        return false;
      }
    }
  }
}
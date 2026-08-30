#pragma once

#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"

namespace Krys::HTML
{
  KRYS_NODISCARD constexpr bool IsCalcFunction(CSSValueId functionId) noexcept
  {
    switch (functionId)
    {
      case CSSValueId::Calc:
      case CSSValueId::WebkitCalc:
      case CSSValueId::Min:
      case CSSValueId::Max:
      case CSSValueId::Clamp:
      case CSSValueId::Pow:
      case CSSValueId::Sqrt:
      case CSSValueId::Hypot:
      case CSSValueId::Sin:
      case CSSValueId::Cos:
      case CSSValueId::Tan:
      case CSSValueId::Exp:
      case CSSValueId::Log:
      case CSSValueId::Asin:
      case CSSValueId::Acos:
      case CSSValueId::Atan:
      case CSSValueId::Atan2:
      case CSSValueId::Abs:
      case CSSValueId::Sign:
      case CSSValueId::Round:
      case CSSValueId::Mod:
      case CSSValueId::Rem:
      case CSSValueId::Progress:
      case CSSValueId::Random:
      case CSSValueId::SiblingCount:
      case CSSValueId::SiblingIndex:
      case CSSValueId::Anchor:
      case CSSValueId::AnchorSize:
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
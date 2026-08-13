#pragma once

#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"

namespace Krys::HTML
{
  KRYS_NODISCARD constexpr bool IsCSSWideKeyword(CSSValueId valueId) noexcept
  {
    switch (valueId)
    {
      case CSSValueId::Initial:
      case CSSValueId::Inherit:
      case CSSValueId::Unset:
      case CSSValueId::Revert:
      case CSSValueId::RevertLayer:
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
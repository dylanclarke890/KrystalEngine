#pragma once

#include "Krystal.Booey/CSS/Values/Enums/CSSValueId.hpp"

namespace krys::boo::css
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
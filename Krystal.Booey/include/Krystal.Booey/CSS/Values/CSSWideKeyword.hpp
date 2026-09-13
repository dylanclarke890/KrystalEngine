#pragma once

#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  KRYS_NODISCARD constexpr bool IsCSSWideKeyword(ValueId valueId) noexcept
  {
    switch (valueId)
    {
      case ValueId::Initial:
      case ValueId::Inherit:
      case ValueId::Unset:
      case ValueId::Revert:
      case ValueId::RevertLayer:
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
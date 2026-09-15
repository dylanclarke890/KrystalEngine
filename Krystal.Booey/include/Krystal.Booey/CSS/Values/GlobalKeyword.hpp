#pragma once

#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  enum class GlobalKeyword : uint8
  {
    Initial,
    Inherit,
    Unset,
    Revert,
    RevertLayer
  };

  KRYS_NODISCARD constexpr bool IsGlobalKeyword(ValueId valueId) noexcept
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

  KRYS_NODISCARD constexpr Maybe<GlobalKeyword> ParseGlobalKeyword(ValueId valueId)
  {
    switch (valueId)
    {
      case ValueId::Initial:     return GlobalKeyword::Initial;
      case ValueId::Inherit:     return GlobalKeyword::Inherit;
      case ValueId::Unset:       return GlobalKeyword::Unset;
      case ValueId::Revert:      return GlobalKeyword::Revert;
      case ValueId::RevertLayer: return GlobalKeyword::RevertLayer;
      default:                   return {};
    }
  }

  KRYS_NODISCARD constexpr ValueId ToValueId(GlobalKeyword keyword) noexcept
  {
    switch (keyword)
    {
      case GlobalKeyword::Initial:     return ValueId::Initial;
      case GlobalKeyword::Inherit:     return ValueId::Inherit;
      case GlobalKeyword::Unset:       return ValueId::Unset;
      case GlobalKeyword::Revert:      return ValueId::Revert;
      case GlobalKeyword::RevertLayer: return ValueId::RevertLayer;
    }

    krys_unreachable();
  }
}
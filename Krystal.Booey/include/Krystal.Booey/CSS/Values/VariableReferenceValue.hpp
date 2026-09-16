#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  class VariableReferenceValue : public Value
  {
  public:
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::VariableReferenceValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsVariableReferenceValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
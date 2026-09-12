#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/Easing/EasingFunction.hpp"

namespace krys::boo::css
{
  class CSSEasingFunctionValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSEasingFunctionValue> Create(EasingFunction function) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSEasingFunctionValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSEasingFunctionValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Easing/EasingFunction.hpp"

namespace Krys::HTML
{
  class CSSEasingFunctionValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSEasingFunctionValue> Create(EasingFunction function) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSEasingFunctionValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSEasingFunctionValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
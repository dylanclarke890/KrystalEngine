#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/TextDecoration/TextShadow.hpp"

namespace krys::boo::css
{
  class CSSTextShadowPropertyValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSTextShadowPropertyValue> Create(TextShadowProperty value) noexcept;
    KRYS_NODISCARD const TextShadowProperty &Value() const noexcept;

  private:
    CSSTextShadowPropertyValue(TextShadowProperty value) noexcept;
    TextShadowProperty _value;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSTextShadowPropertyValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSTextShadowPropertyValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
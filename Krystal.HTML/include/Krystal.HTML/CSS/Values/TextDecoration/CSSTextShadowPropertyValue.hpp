#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/TextDecoration/TextShadow.hpp"

namespace Krys::HTML
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSTextShadowPropertyValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSTextShadowPropertyValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
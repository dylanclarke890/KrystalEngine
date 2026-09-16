#pragma once

#include "Krystal.Booey/CSS/Values/TextDecoration/TextShadow.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  class TextShadowPropertyValue final : public Value
  {
  private:
    TextShadowProperty _value;

    TextShadowPropertyValue(TextShadowProperty value) noexcept;

  public:
    KRYS_NODISCARD static Ref<TextShadowPropertyValue> Create(TextShadowProperty value) noexcept;

    KRYS_NODISCARD const TextShadowProperty &Value() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::TextShadowPropertyValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsTextShadowPropertyValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
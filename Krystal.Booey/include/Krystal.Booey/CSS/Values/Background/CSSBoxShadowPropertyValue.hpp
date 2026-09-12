#pragma once

#include "Krystal.Booey/CSS/Values/Background/BoxShadowProperty.hpp"
#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSBoxShadowPropertyValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSBoxShadowPropertyValue> Create(BoxShadowProperty boxShadow) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSBoxShadowPropertyValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSBoxShadowPropertyValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
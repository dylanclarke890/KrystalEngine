#pragma once

#include "Krystal.HTML/CSS/Values/Background/BoxShadowProperty.hpp"
#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSBoxShadowPropertyValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSBoxShadowPropertyValue> Create(BoxShadowProperty boxShadow) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSBoxShadowPropertyValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSBoxShadowPropertyValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Booey/CSS/Values/ColorAdjust/ColorScheme.hpp"

namespace krys::boo::css
{
  class CSSColorSchemeValue : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSColorSchemeValue> Create(ColorScheme scheme) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSColorSchemeValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSColorSchemeValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
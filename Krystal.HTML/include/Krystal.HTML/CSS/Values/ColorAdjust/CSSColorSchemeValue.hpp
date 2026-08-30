#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.HTML/CSS/Values/ColorAdjust/ColorScheme.hpp"

namespace Krys::HTML
{
  class CSSColorSchemeValue : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSColorSchemeValue> Create(ColorScheme scheme) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSColorSchemeValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSColorSchemeValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
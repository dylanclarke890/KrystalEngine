#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSRatio.hpp"

namespace Krys::HTML
{
  class CSSRatioValue : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSRatioValue> Create(Ratio ratio) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSRatioValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSRatioValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
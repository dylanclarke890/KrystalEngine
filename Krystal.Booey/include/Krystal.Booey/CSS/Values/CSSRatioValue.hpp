#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSRatio.hpp"

namespace krys::boo::css
{
  class CSSRatioValue : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSRatioValue> Create(Ratio ratio) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSRatioValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSRatioValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
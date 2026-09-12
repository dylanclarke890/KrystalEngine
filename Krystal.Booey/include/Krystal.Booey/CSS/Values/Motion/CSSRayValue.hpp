#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/Motion/RayFunction.hpp"

namespace krys::boo::css
{
  class CSSRayValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSRayValue> Create(RayFunction ray) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSRayValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSRayValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
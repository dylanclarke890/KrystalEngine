#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Motion/RayFunction.hpp"

namespace Krys::HTML
{
  class CSSRayValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSRayValue> Create(RayFunction ray) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSRayValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSRayValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
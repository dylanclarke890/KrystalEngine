#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Quad.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSBackgroundRepeatValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSBackgroundRepeatValue> Create(CSSValueId repeatX, CSSValueId repeatY) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSBackgroundRepeatValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSBackgroundRepeatValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
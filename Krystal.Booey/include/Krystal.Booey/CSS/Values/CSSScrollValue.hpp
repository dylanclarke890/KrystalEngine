#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSScrollValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSScrollValue> Create(RefPtr<CSSValue> scroller, RefPtr<CSSValue> axis) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSScrollValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSScrollValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
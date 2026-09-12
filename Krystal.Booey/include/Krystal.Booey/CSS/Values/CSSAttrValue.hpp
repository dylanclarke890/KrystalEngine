#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSAttrValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSAttrValue> Create(CSSOMStringAtom attr, RefPtr<CSSValue> fallback) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSAttrValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSAttrValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
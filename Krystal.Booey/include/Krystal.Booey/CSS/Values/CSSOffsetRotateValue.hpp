#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSOffsetRotateValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSOffsetRotateValue> Create(RefPtr<CSSValue> modifier,
                                                           RefPtr<CSSValue> angle) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSOffsetRotateValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSOffsetRotateValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
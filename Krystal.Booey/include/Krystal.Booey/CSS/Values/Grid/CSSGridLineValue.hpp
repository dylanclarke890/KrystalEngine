#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSPrimitiveValue;

  class CSSGridLineValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSGridLineValue> Create(RefPtr<CSSPrimitiveValue> spanValue,
                                                       RefPtr<CSSPrimitiveValue> numericValue,
                                                       RefPtr<CSSPrimitiveValue> gridLineName) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSGridLineValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSGridLineValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
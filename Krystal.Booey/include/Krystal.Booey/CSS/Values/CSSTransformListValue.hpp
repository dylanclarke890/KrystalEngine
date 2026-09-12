#pragma once

#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"

namespace krys::boo::css
{
  class CSSTransformListValue final : public CSSValueContainingVector
  {
  public:
    KRYS_NODISCARD static Ref<CSSTransformListValue> Create(CSSValueListBuilder) noexcept;

    KRYS_NODISCARD static Ref<CSSTransformListValue> Create(Ref<CSSValue>) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSTransformListValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSTransformListValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"

namespace Krys::HTML
{
  class CSSTransformListValue final : public CSSValueContainingVector
  {
  public:
    KRYS_NODISCARD static Ref<CSSTransformListValue> Create(CSSValueListBuilder) noexcept;

    KRYS_NODISCARD static Ref<CSSTransformListValue> Create(Ref<CSSValue>) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSTransformListValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSTransformListValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
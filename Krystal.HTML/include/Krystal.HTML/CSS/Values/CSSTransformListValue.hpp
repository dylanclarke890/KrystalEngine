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
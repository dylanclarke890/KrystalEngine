#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Rect.hpp"

namespace krys::boo::css
{
  class CSSRectValue : public CSSValue
  {
  private:
    Rect _rect;

    explicit CSSRectValue(Rect rect) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSRectValue> Create(Rect rect) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSRectValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSRectValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
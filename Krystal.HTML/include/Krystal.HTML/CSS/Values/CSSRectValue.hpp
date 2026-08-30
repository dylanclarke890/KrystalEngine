#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/Rect.hpp"

namespace Krys::HTML
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSRectValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSRectValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
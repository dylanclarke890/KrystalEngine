#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/Quad.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSBackgroundRepeatValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSBackgroundRepeatValue> Create(CSSValueId repeatX, CSSValueId repeatY) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSBackgroundRepeatValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSBackgroundRepeatValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
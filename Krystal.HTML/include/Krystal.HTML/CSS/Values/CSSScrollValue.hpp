#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSScrollValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSScrollValue> Create(RefPtr<CSSValue> scroller, RefPtr<CSSValue> axis) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSScrollValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSScrollValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSAttrValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSAttrValue> Create(CSSOMStringAtom attr, RefPtr<CSSValue> fallback) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSAttrValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSAttrValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
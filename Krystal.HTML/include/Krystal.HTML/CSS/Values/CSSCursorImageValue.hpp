#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSCursorImageValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSCursorImageValue> Create(Ref<CSSValue> &&image,
                                                          Ref<CSSValue> &&hotSpot) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSCursorImageValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSCursorImageValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
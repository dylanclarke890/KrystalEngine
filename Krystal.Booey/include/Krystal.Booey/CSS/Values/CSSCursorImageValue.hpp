#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSCursorImageValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSCursorImageValue> Create(Ref<CSSValue> &&image,
                                                          Ref<CSSValue> &&hotSpot) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSCursorImageValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSCursorImageValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
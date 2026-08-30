#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSQuadValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSQuadValue> Create(Ref<CSSValue> value) noexcept;

    KRYS_NODISCARD static Ref<CSSQuadValue> Create(Ref<CSSValue> first, Ref<CSSValue> second) noexcept;

    KRYS_NODISCARD static Ref<CSSQuadValue> Create(Ref<CSSValue> first, Ref<CSSValue> second,
                                                   Ref<CSSValue> third) noexcept;

    KRYS_NODISCARD static Ref<CSSQuadValue> Create(Ref<CSSValue> first, Ref<CSSValue> second,
                                                   Ref<CSSValue> third, Ref<CSSValue> fourth) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSQuadValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSQuadValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
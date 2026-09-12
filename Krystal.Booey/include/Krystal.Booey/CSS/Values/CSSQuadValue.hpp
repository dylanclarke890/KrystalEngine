#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSQuadValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSQuadValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
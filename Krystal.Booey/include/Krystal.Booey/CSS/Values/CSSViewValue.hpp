#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSViewValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSViewValue> Create() noexcept;

    KRYS_NODISCARD static Ref<CSSViewValue> Create(Ref<CSSValue> axis, Ref<CSSValue> startInset,
                                                   Ref<CSSValue> endInset) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSViewValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSViewValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
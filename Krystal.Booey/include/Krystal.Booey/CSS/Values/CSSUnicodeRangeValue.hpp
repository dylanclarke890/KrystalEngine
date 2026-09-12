#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSUnicodeRangeValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSUnicodeRangeValue> Create(char32 start, char32 end) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSUnicodeRangeValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSUnicodeRangeValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
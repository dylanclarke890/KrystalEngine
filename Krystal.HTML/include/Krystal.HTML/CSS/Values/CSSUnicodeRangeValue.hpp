#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSUnicodeRangeValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSUnicodeRangeValue> Create(char32 start, char32 end) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSUnicodeRangeValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSUnicodeRangeValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSPrimitiveValue;

  class CSSGridLineValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSGridLineValue> Create(RefPtr<CSSPrimitiveValue> spanValue,
                                                       RefPtr<CSSPrimitiveValue> numericValue,
                                                       RefPtr<CSSPrimitiveValue> gridLineName) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSGridLineValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSGridLineValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
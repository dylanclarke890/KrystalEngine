#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"

namespace Krys::HTML
{
  class CSSPrimitiveValue : public CSSValue
  {
  private:
    CSSValueId _valueId : BitCount<CSSValueId>() {CSSValueId::Invalid};

  public:
    CSSPrimitiveValue(CSSValueId valueId) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSPrimitiveValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSPrimitiveValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
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
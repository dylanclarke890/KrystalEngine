#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"

namespace Krys::HTML
{
  CSSPrimitiveValue::CSSPrimitiveValue(CSSValueId valueId) noexcept
      : CSSValue(CSSValueType::Primitive), _valueId(valueId)
  {
  }
}
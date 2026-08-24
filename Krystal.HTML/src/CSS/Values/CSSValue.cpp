#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"

namespace Krys::HTML
{
  CSSValueId CSSValue::ValueId() const noexcept
  {
    auto *value = DynamicDowncast<CSSPrimitiveValue>(*this);
    return value ? value->ValueId() : CSSValueId::Invalid;
  }
}
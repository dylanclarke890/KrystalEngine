#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"

namespace krys::boo::css
{
  CSSValueId CSSValue::ValueId() const noexcept
  {
    auto *value = DynamicDowncast<CSSPrimitiveValue>(*this);
    return value ? value->ValueId() : CSSValueId::Invalid;
  }
}
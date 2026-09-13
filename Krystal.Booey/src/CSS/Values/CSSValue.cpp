#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"

namespace krys::boo::css
{
  ValueId CSSValue::ValueId() const noexcept
  {
    auto *value = DynamicDowncast<CSSPrimitiveValue>(*this);
    return value ? value->ValueId() : ValueId::Invalid;
  }
}
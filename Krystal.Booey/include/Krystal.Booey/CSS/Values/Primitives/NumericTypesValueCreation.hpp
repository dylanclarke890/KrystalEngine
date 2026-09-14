#pragma once

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  template <NumericRaw CSSType>
  struct CSSValueCreation<CSSType>
  {
    Ref<Value> operator()(ValuePool &, const CSSType &raw)
    {
      return PrimitiveValue::Create(raw.Value, ToUnitType(raw.Unit));
    }
  };

  template <Calc CSSType>
  struct CSSValueCreation<CSSType>
  {
    Ref<Value> operator()(ValuePool &, const CSSType &calc)
    {
      return PrimitiveValue::Create(ShareRef(*calc.get()));
    }
  };
}
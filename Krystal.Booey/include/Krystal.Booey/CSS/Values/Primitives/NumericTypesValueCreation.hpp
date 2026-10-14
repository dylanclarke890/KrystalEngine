#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"

namespace krys::boo::css
{
  template <NumericRaw CSSType>
  struct ValueCreation<CSSType>
  {
    KRYS_NODISCARD Ref<Value> operator()(ValuePool &, const CSSType &raw) noexcept
    {
      return PrimitiveValue::Create(raw.Value, ToUnitType(raw.Unit));
    }
  };

  template <Calc CSSType>
  struct ValueCreation<CSSType>
  {
    KRYS_NODISCARD Ref<Value> operator()(ValuePool &, const CSSType &calc) noexcept
    {
      return PrimitiveValue::Create(calc.ProtectedCalc());
    }
  };
}
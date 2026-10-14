#include "Krystal.Booey/Style/Values/CounterStyles/CounterStyle.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  CounterStyle CSSValueConversion<CounterStyle>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return CounterStyle {{css::CSSOMStringAtom {css::ToString(css::ValueId::Decimal)}}};
    }

    return CounterStyle {{css::CSSOMStringAtom {primitiveValue->StringValue()}}};
  }
}

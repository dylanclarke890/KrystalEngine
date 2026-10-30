#include "Krystal.Booey/Style/Values/Animations/SingleAnimationName.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  SingleAnimationName CSSValueConversion<SingleAnimationName>::operator()(BuilderState &state,
                                                                          const css::Value &value) noexcept
  {
    const auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return SingleAnimationName {css::keywords::None {}};
    }

    if (primitiveValue->ValueId() == css::ValueId::None)
    {
      return SingleAnimationName {css::keywords::None {}};
    }

    return SingleAnimationName {ScopedName {css::CSSOMStringAtom {primitiveValue->StringValue()},
                                            state.StyleScopeOrdinal(), primitiveValue->IsCustomIdent()}};
  }
}

#include "Krystal.Booey/Style/Values/ViewTransitions/ViewTransitionName.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  ViewTransitionName CSSValueConversion<ViewTransitionName>::operator()(BuilderState &state,
                                                                        const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::None {};
    }

    switch (primitiveValue->ValueId())
    {
      case css::ValueId::None:         return css::keywords::None {};
      case css::ValueId::Auto:         return {css::keywords::Auto {}, state.StyleScopeOrdinal()};
      case css::ValueId::MatchElement: return {css::keywords::MatchElement {}, state.StyleScopeOrdinal()};
      default:                         break;
    }

    return {css::CustomIdentifier {css::CSSOMStringAtom {primitiveValue->StringValue()}},
            state.StyleScopeOrdinal()};
  }
}

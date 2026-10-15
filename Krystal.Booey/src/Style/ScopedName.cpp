#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  ScopedName CSSValueConversion<ScopedName>::operator()(BuilderState &state,
                                                        const css::PrimitiveValue &primitiveValue) noexcept
  {
    if (!primitiveValue.IsCustomIdent())
    {
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return ScopedName {css::CSSOMStringAtom::Null()};
    }

    return {.Name = css::CSSOMStringAtom {primitiveValue.StringValue()},
            .ScopeOrdinal = state.StyleScopeOrdinal()};
  }

  ScopedName CSSValueConversion<ScopedName>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    const auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return ScopedName {css::CSSOMStringAtom::Null()};
    }

    return (*this)(state, *primitiveValue);
  }
}

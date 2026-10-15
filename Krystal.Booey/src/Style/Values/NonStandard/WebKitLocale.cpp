#include "Krystal.Booey/Style/Values/NonStandard/WebKitLocale.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  WebkitLocale CSSValueConversion<WebkitLocale>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Auto {};
    }

    switch (primitiveValue->ValueId())
    {
      case css::ValueId::Invalid: return css::CSSOMStringAtom {primitiveValue->StringValue()};
      case css::ValueId::Auto:    return css::keywords::Auto {};
      default:                    state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Auto {};
    }
  }
}

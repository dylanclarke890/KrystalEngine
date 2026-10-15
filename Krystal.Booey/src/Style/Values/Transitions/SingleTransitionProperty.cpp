#include "Krystal.Booey/Style/Values/Transitions/SingleTransitionProperty.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  SingleTransitionProperty
    CSSValueConversion<SingleTransitionProperty>::operator()(BuilderState &state,
                                                             const css::Value &value) noexcept
  {
    const auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::All {};
    }

    switch (primitiveValue->ValueId())
    {
      case css::ValueId::All:  return css::keywords::All {};
      case css::ValueId::None: return css::keywords::None {};
      default:                 break;
    }

    auto propertyID = primitiveValue->PropertyId();
    if (propertyID == css::PropertyId::Invalid)
    {
      return css::CustomIdentifier {css::CSSOMStringAtom {primitiveValue->StringValue()}};
    }

    return propertyID;
  }
}

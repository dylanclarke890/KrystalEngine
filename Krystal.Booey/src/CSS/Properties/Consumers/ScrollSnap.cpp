#include "Krystal.Booey/CSS/Properties/Consumers/ScrollSnap.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<CSSValue> ConsumeScrollSnapType(TokenRange &tokens, PropertyParserState &) noexcept
  {
    // <'scroll-snap-type'> = none | [ x | y | block | inline | both ] [ mandatory | proximity
    // ]?@(default=proximity) https://drafts.csswg.org/css-scroll-snap-1/#scroll-snap-type

    auto firstValue = ConsumeIdentRaw<ValueId::None, ValueId::X, ValueId::Y, ValueId::Block,
                                      ValueId::Inline, ValueId::Both>(tokens);
    if (!firstValue)
    {
      return nullptr;
    }

    if (*firstValue == ValueId::None)
    {
      return CSSPrimitiveValue::Create(ValueId::None);
    }

    // We only add the second value if it is not the initial value as described in specification
    // so that serialization of this CSSValueList produces the canonical serialization.

    auto secondValue = ConsumeIdentRaw<ValueId::Proximity, ValueId::Mandatory>(tokens);
    if (secondValue.value_or(ValueId::Proximity) == ValueId::Proximity)
    {
      return CSSPrimitiveValue::Create(*firstValue);
    }

    return CSSValueList::CreateSpaceSeparated(CSSPrimitiveValue::Create(*firstValue),
                                              CSSPrimitiveValue::Create(ValueId::Mandatory));
  }
}
#include "Krystal.Booey/CSS/Properties/Consumers/ScrollSnap.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"

namespace krys::boo::css::CSSPropertyParserHelpers
{
  RefPtr<CSSValue> ConsumeScrollSnapType(TokenRange &tokens, CSSPropertyParserState &) noexcept
  {
    // <'scroll-snap-type'> = none | [ x | y | block | inline | both ] [ mandatory | proximity
    // ]?@(default=proximity) https://drafts.csswg.org/css-scroll-snap-1/#scroll-snap-type

    auto firstValue = ConsumeIdentRaw<CSSValueId::None, CSSValueId::X, CSSValueId::Y, CSSValueId::Block,
                                      CSSValueId::Inline, CSSValueId::Both>(tokens);
    if (!firstValue)
    {
      return nullptr;
    }

    if (*firstValue == CSSValueId::None)
    {
      return CSSPrimitiveValue::Create(CSSValueId::None);
    }

    // We only add the second value if it is not the initial value as described in specification
    // so that serialization of this CSSValueList produces the canonical serialization.

    auto secondValue = ConsumeIdentRaw<CSSValueId::Proximity, CSSValueId::Mandatory>(tokens);
    if (secondValue.value_or(CSSValueId::Proximity) == CSSValueId::Proximity)
    {
      return CSSPrimitiveValue::Create(*firstValue);
    }

    return CSSValueList::CreateSpaceSeparated(CSSPrimitiveValue::Create(*firstValue),
                                              CSSPrimitiveValue::Create(CSSValueId::Mandatory));
  }
}
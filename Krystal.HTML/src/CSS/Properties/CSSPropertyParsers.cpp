#include "Krystal.HTML/CSS/Properties/CSSPropertyParsers.hpp"

#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/TimeDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/ResolutionDefinitions.hpp"
#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumeric.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericRange.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericRaw.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML {

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSAllValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    // initial | inherit | unset | revert | revert-layer
    return ConsumeIdent<CSSValueId::Initial, CSSValueId::Inherit, CSSValueId::Unset, CSSValueId::Revert, CSSValueId::RevertLayer>(range);
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSMarginValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    return ParseCSSMarginTopValue(range, state);
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSPaddingValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    return ParseCSSPaddingTopValue(range, state);
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSMarginBottomValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    if (auto result = CSSPrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(range, state))
        return result;
    // auto
    if (auto result = ConsumeIdent<CSSValueId::Auto>(range))
        return result;
    
    return nullptr;
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSMarginLeftValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    if (auto result = CSSPrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(range, state))
        return result;
    // auto
    if (auto result = ConsumeIdent<CSSValueId::Auto>(range))
        return result;
    
    return nullptr;
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSMarginRightValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    if (auto result = CSSPrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(range, state))
        return result;
    // auto
    if (auto result = ConsumeIdent<CSSValueId::Auto>(range))
        return result;
    
    return nullptr;
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSMarginTopValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    if (auto result = CSSPrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(range, state))
        return result;
    // auto
    if (auto result = ConsumeIdent<CSSValueId::Auto>(range))
        return result;
    
    return nullptr;
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSPaddingBottomValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    return CSSPrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(range, state);
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSPaddingLeftValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    return CSSPrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(range, state);
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSPaddingRightValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    return CSSPrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(range, state);
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSPaddingTopValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    return CSSPrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(range, state);
}

}

#pragma once

#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
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
    
    // initial
    switch (auto keyword = range.Peek().ValueId(); keyword) {
    case CSSValueId::Initial:
        range.Discard();
        range.DiscardWhitespace();
        return CreateRef<CSSPrimitiveValue>(CSSValueId::Initial);
    default:
        break;
    }
    // inherit
    switch (auto keyword = range.Peek().ValueId(); keyword) {
    case CSSValueId::Inherit:
        range.Discard();
        range.DiscardWhitespace();
        return CreateRef<CSSPrimitiveValue>(CSSValueId::Inherit);
    default:
        break;
    }
    // unset
    switch (auto keyword = range.Peek().ValueId(); keyword) {
    case CSSValueId::Unset:
        range.Discard();
        range.DiscardWhitespace();
        return CreateRef<CSSPrimitiveValue>(CSSValueId::Unset);
    default:
        break;
    }
    // revert
    switch (auto keyword = range.Peek().ValueId(); keyword) {
    case CSSValueId::Revert:
        range.Discard();
        range.DiscardWhitespace();
        return CreateRef<CSSPrimitiveValue>(CSSValueId::Revert);
    default:
        break;
    }
    // revert-layer
    switch (auto keyword = range.Peek().ValueId(); keyword) {
    case CSSValueId::RevertLayer:
        range.Discard();
        range.DiscardWhitespace();
        return CreateRef<CSSPrimitiveValue>(CSSValueId::RevertLayer);
    default:
        break;
    }
    
    return nullptr;
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSMarginValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    return ParseCSSMarginTopValue(range, state);
    
    return nullptr;
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSPaddingValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    return ParseCSSPaddingTopValue(range, state);
    
    return nullptr;
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSMarginBottomValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    if (auto result = CSSPrimitiveValueResolver<PrimitiveNumeric<LengthPercentageRaw<>>>::ConsumeAndResolve(range, state))
        return result;
    // auto
    switch (auto keyword = range.Peek().ValueId(); keyword) {
    case CSSValueId::Auto:
        range.Discard();
        range.DiscardWhitespace();
        return CreateRef<CSSPrimitiveValue>(CSSValueId::Auto);
    default:
        break;
    }
    
    return nullptr;
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSMarginLeftValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    if (auto result = CSSPrimitiveValueResolver<PrimitiveNumeric<LengthPercentageRaw<>>>::ConsumeAndResolve(range, state))
        return result;
    // auto
    switch (auto keyword = range.Peek().ValueId(); keyword) {
    case CSSValueId::Auto:
        range.Discard();
        range.DiscardWhitespace();
        return CreateRef<CSSPrimitiveValue>(CSSValueId::Auto);
    default:
        break;
    }
    
    return nullptr;
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSMarginRightValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    if (auto result = CSSPrimitiveValueResolver<PrimitiveNumeric<LengthPercentageRaw<>>>::ConsumeAndResolve(range, state))
        return result;
    // auto
    switch (auto keyword = range.Peek().ValueId(); keyword) {
    case CSSValueId::Auto:
        range.Discard();
        range.DiscardWhitespace();
        return CreateRef<CSSPrimitiveValue>(CSSValueId::Auto);
    default:
        break;
    }
    
    return nullptr;
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSMarginTopValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    if (auto result = CSSPrimitiveValueResolver<PrimitiveNumeric<LengthPercentageRaw<>>>::ConsumeAndResolve(range, state))
        return result;
    // auto
    switch (auto keyword = range.Peek().ValueId(); keyword) {
    case CSSValueId::Auto:
        range.Discard();
        range.DiscardWhitespace();
        return CreateRef<CSSPrimitiveValue>(CSSValueId::Auto);
    default:
        break;
    }
    
    return nullptr;
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSPaddingBottomValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    return CSSPrimitiveValueResolver<PrimitiveNumeric<LengthPercentageRaw<CSSRange{0, CSSRange::Inf}>>>::ConsumeAndResolve(range, state);
    
    return nullptr;
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSPaddingLeftValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    return CSSPrimitiveValueResolver<PrimitiveNumeric<LengthPercentageRaw<CSSRange{0, CSSRange::Inf}>>>::ConsumeAndResolve(range, state);
    
    return nullptr;
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSPaddingRightValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    return CSSPrimitiveValueResolver<PrimitiveNumeric<LengthPercentageRaw<CSSRange{0, CSSRange::Inf}>>>::ConsumeAndResolve(range, state);
    
    return nullptr;
}

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSPaddingTopValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
{
    auto range = tokens;
    
    return CSSPrimitiveValueResolver<PrimitiveNumeric<LengthPercentageRaw<CSSRange{0, CSSRange::Inf}>>>::ConsumeAndResolve(range, state);
    
    return nullptr;
}

}

#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML {

class CSSTokenRange;
struct CSSPropertyParserState;
class CSSValue;

KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSAllValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;
KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSMarginValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;
KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSPaddingValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;
KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSMarginBottomValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;
KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSMarginLeftValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;
KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSMarginRightValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;
KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSMarginTopValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;
KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSPaddingBottomValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;
KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSPaddingLeftValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;
KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSPaddingRightValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;
KRYS_NODISCARD Maybe<RefPtr<CSSValue>> ParseCSSPaddingTopValue(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;

}

#pragma once

#include "Krystal.Booey/CSS/Parser/Context/ParserMode.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // https://drafts.csswg.org/css-counter-styles-3/

    // MARK: <counter-style> consumer
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterStyle(TokenRange &tokens,
                                                        CSSPropertyParserState &state) noexcept;

    // MARK: @counter-style consumer
    KRYS_NODISCARD CSSOMStringAtom ConsumeCounterStyleNameInPrelude(
      TokenRange &tokens, ParserMode mode = ParserMode::HTMLStandard) noexcept;

    // MARK: @counter-style descriptor consumers
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterStyleName(TokenRange &tokens,
                                                            CSSPropertyParserState &state) noexcept;
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterStyleSystem(TokenRange &tokens,
                                                              CSSPropertyParserState &state) noexcept;
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterStyleRange(TokenRange &tokens,
                                                             CSSPropertyParserState &state) noexcept;
    KRYS_NODISCARD RefPtr<CSSValue>
      ConsumeCounterStyleAdditiveSymbols(TokenRange &tokens, CSSPropertyParserState &state) noexcept;
  }
}
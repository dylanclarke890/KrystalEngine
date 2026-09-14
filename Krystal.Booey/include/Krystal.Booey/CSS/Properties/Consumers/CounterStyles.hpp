#pragma once

#include "Krystal.Booey/CSS/Parser/Context/ParserMode.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // https://drafts.csswg.org/css-counter-styles-3/

    // MARK: <counter-style> consumer
    KRYS_NODISCARD RefPtr<Value> ConsumeCounterStyle(TokenRange &tokens,
                                                        PropertyParserState &state) noexcept;

    // MARK: @counter-style consumer
    KRYS_NODISCARD CSSOMStringAtom ConsumeCounterStyleNameInPrelude(
      TokenRange &tokens, ParserMode mode = ParserMode::HTMLStandard) noexcept;

    // MARK: @counter-style descriptor consumers
    KRYS_NODISCARD RefPtr<Value> ConsumeCounterStyleName(TokenRange &tokens,
                                                            PropertyParserState &state) noexcept;
    KRYS_NODISCARD RefPtr<Value> ConsumeCounterStyleSystem(TokenRange &tokens,
                                                              PropertyParserState &state) noexcept;
    KRYS_NODISCARD RefPtr<Value> ConsumeCounterStyleRange(TokenRange &tokens,
                                                             PropertyParserState &state) noexcept;
    KRYS_NODISCARD RefPtr<Value>
      ConsumeCounterStyleAdditiveSymbols(TokenRange &tokens, PropertyParserState &state) noexcept;
  }
}
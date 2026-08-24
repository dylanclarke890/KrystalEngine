#pragma once

#include "Krystal.HTML/CSS/Parser/Enums/CSSParserMode.hpp"
#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // https://drafts.csswg.org/css-counter-styles-3/

    // MARK: <counter-style> consumer
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterStyle(CSSTokenRange &tokens,
                                                        CSSPropertyParserState &state) noexcept;

    // MARK: @counter-style consumer
    KRYS_NODISCARD CSSOMStringAtom ConsumeCounterStyleNameInPrelude(
      CSSTokenRange &tokens, CSSParserMode mode = CSSParserMode::HTMLStandard) noexcept;

    // MARK: @counter-style descriptor consumers
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterStyleName(CSSTokenRange &tokens,
                                                            CSSPropertyParserState &state) noexcept;
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterStyleSystem(CSSTokenRange &tokens,
                                                              CSSPropertyParserState &state) noexcept;
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterStyleRange(CSSTokenRange &tokens,
                                                             CSSPropertyParserState &state) noexcept;
    KRYS_NODISCARD RefPtr<CSSValue>
      ConsumeCounterStyleAdditiveSymbols(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;
  }
}
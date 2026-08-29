#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  struct CSSPropertyParserState;
  class CSSValue;

  namespace CSSPropertyParserHelpers
  {
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterReset(CSSTokenRange &tokens,
                                                        CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterIncrement(CSSTokenRange &tokens,
                                                            CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterSet(CSSTokenRange &tokens,
                                                      CSSPropertyParserState &state) noexcept;
  }
}
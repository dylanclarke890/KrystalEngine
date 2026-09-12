#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct CSSPropertyParserState;
  class CSSValue;

  namespace CSSPropertyParserHelpers
  {
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterReset(TokenRange &tokens,
                                                        CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterIncrement(TokenRange &tokens,
                                                            CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterSet(TokenRange &tokens,
                                                      CSSPropertyParserState &state) noexcept;
  }
}
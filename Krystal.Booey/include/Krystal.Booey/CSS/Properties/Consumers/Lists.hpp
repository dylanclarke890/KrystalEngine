#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct PropertyParserState;
  class CSSValue;

  namespace PropertyParserHelpers
  {
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterReset(TokenRange &tokens,
                                                        PropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterIncrement(TokenRange &tokens,
                                                            PropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCounterSet(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept;
  }
}
#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct PropertyParserState;
  class Value;

  namespace PropertyParserHelpers
  {
    KRYS_NODISCARD RefPtr<Value> ConsumeCounterReset(TokenRange &tokens,
                                                        PropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<Value> ConsumeCounterIncrement(TokenRange &tokens,
                                                            PropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<Value> ConsumeCounterSet(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept;
  }
}
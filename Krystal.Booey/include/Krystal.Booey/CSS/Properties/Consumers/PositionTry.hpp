#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct PropertyParserState;
  class Value;

  namespace PropertyParserHelpers
  {
    // MARK: <'position-try-fallbacks'>
    // https://drafts.csswg.org/css-anchor-position-1/#propdef-position-try-fallbacks
    KRYS_NODISCARD RefPtr<Value> ConsumePositionTryFallbacks(TokenRange &tokens,
                                                                PropertyParserState &state) noexcept;
  }
}
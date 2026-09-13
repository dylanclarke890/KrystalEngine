#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct PropertyParserState;
  class CSSValue;

  namespace PropertyParserHelpers
  {
    // MARK: <'position-try-fallbacks'>
    // https://drafts.csswg.org/css-anchor-position-1/#propdef-position-try-fallbacks
    KRYS_NODISCARD RefPtr<CSSValue> ConsumePositionTryFallbacks(TokenRange &tokens,
                                                                PropertyParserState &state) noexcept;
  }
}
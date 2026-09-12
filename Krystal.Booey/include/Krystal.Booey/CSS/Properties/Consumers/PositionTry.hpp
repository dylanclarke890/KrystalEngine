#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct CSSPropertyParserState;
  class CSSValue;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <'position-try-fallbacks'>
    // https://drafts.csswg.org/css-anchor-position-1/#propdef-position-try-fallbacks
    KRYS_NODISCARD RefPtr<CSSValue> ConsumePositionTryFallbacks(TokenRange &tokens,
                                                                CSSPropertyParserState &state) noexcept;
  }
}
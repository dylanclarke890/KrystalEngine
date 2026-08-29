#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  struct CSSPropertyParserState;
  class CSSValue;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <'position-try-fallbacks'>
    // https://drafts.csswg.org/css-anchor-position-1/#propdef-position-try-fallbacks
    KRYS_NODISCARD RefPtr<CSSValue> ConsumePositionTryFallbacks(CSSTokenRange &tokens,
                                                                CSSPropertyParserState &state) noexcept;
  }
}
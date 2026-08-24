#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    /// @grammar <'margin-trim'>
    /// @see https://drafts.csswg.org/css-box/#margin-trim
    // https://drafts.csswg.org/css-box/#margin-trim
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeMarginTrim(CSSTokenRange &tokens,
                                                      CSSPropertyParserState &state) noexcept;
  }
}
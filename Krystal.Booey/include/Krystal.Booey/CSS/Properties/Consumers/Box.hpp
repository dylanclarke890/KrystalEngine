#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    /// @grammar <'margin-trim'>
    /// @see https://drafts.csswg.org/css-box/#margin-trim
    // https://drafts.csswg.org/css-box/#margin-trim
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeMarginTrim(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept;
  }
}
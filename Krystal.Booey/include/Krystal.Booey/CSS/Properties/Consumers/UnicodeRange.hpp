#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // https://drafts.csswg.org/css-syntax-3/#typedef-unicode-range-token

    // MARK: <unicode-range-token> consuming (value)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeUnicodeRangeToken(TokenRange &tokens) noexcept;
  }
}
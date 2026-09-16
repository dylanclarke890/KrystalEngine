#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // https://drafts.csswg.org/css-syntax-3/#typedef-unicode-range-token

    // MARK: <unicode-range-token> consuming (value)
    KRYS_NODISCARD RefPtr<Value> ConsumeUnicodeRangeToken(TokenRange &tokens) noexcept;
  }
}
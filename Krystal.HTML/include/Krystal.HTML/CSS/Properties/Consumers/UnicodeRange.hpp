#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // https://drafts.csswg.org/css-syntax-3/#typedef-unicode-range-token

    // MARK: <unicode-range-token> consuming (value)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeUnicodeRangeToken(CSSTokenRange &tokens) noexcept;
  }
}
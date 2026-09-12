#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <'will-change'> consuming
    // https://drafts.csswg.org/css-will-change/#propdef-will-change
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeWillChange(TokenRange &tokens,
                                                      CSSPropertyParserState &state) noexcept;
  }
}
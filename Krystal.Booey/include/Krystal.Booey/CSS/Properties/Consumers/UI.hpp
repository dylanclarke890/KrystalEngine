#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <'cursor'> consuming
    // https://drafts.csswg.org/css-ui/#propdef-cursor
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCursor(TokenRange &tokens,
                                                  CSSPropertyParserState &state) noexcept;
  }
}
#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // MARK: <'cursor'> consuming
    // https://drafts.csswg.org/css-ui/#propdef-cursor
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCursor(TokenRange &tokens,
                                                  PropertyParserState &state) noexcept;
  }
}
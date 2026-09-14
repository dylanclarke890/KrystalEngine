#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // MARK: <'cursor'> consuming
    // https://drafts.csswg.org/css-ui/#propdef-cursor
    KRYS_NODISCARD RefPtr<Value> ConsumeCursor(TokenRange &tokens,
                                                  PropertyParserState &state) noexcept;
  }
}
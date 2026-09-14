#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // MARK: <'will-change'> consuming
    // https://drafts.csswg.org/css-will-change/#propdef-will-change
    KRYS_NODISCARD RefPtr<Value> ConsumeWillChange(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept;
  }
}
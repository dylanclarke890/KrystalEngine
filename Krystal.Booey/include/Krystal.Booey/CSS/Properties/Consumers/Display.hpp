#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    /// @grammar <'display'> consuming
    /// @see https://drafts.csswg.org/css-display/#propdef-display
    KRYS_NODISCARD RefPtr<Value> ConsumeDisplay(TokenRange &tokens,
                                                   PropertyParserState &state) noexcept;
  }
}
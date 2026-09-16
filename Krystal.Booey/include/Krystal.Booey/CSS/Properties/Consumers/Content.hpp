#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    /// @grammar <'quotes'>
    /// @see https://drafts.csswg.org/css-content-3/#propdef-quotes
    KRYS_NODISCARD RefPtr<Value> ConsumeQuotes(TokenRange &tokens,
                                                  PropertyParserState &state) noexcept;

    /// @grammar <'content'>
    /// @see https://drafts.csswg.org/css-content-3/#propdef-content
    KRYS_NODISCARD RefPtr<Value> ConsumeContent(TokenRange &tokens,
                                                   PropertyParserState &state) noexcept;
  }
}
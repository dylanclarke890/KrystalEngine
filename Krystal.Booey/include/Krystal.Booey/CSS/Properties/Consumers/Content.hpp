#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    /// @grammar <'quotes'>
    /// @see https://drafts.csswg.org/css-content-3/#propdef-quotes
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeQuotes(TokenRange &tokens,
                                                  CSSPropertyParserState &state) noexcept;

    /// @grammar <'content'>
    /// @see https://drafts.csswg.org/css-content-3/#propdef-content
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeContent(TokenRange &tokens,
                                                   CSSPropertyParserState &state) noexcept;
  }
}
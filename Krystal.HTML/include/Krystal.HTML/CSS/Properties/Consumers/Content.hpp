#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    /// @grammar <'quotes'>
    /// @see https://drafts.csswg.org/css-content-3/#propdef-quotes
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeQuotes(CSSTokenRange &tokens,
                                                  CSSPropertyParserState &state) noexcept;

    /// @grammar <'content'>
    /// @see https://drafts.csswg.org/css-content-3/#propdef-content
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeContent(CSSTokenRange &tokens,
                                                   CSSPropertyParserState &state) noexcept;
  }
}
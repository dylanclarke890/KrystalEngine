#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    /// @grammar <'display'> consuming
    /// @see https://drafts.csswg.org/css-display/#propdef-display
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeDisplay(CSSTokenRange &tokens,
                                                   CSSPropertyParserState &state) noexcept;
  }
}
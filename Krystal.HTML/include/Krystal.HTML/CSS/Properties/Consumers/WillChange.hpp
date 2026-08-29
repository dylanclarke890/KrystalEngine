#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <'will-change'> consuming
    // https://drafts.csswg.org/css-will-change/#propdef-will-change
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeWillChange(CSSTokenRange &tokens,
                                                      CSSPropertyParserState &state) noexcept;
  }
}
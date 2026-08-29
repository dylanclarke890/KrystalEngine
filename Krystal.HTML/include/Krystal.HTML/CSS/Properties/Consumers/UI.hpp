#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <'cursor'> consuming
    // https://drafts.csswg.org/css-ui/#propdef-cursor
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeCursor(CSSTokenRange &tokens,
                                                  CSSPropertyParserState &state) noexcept;
  }
}
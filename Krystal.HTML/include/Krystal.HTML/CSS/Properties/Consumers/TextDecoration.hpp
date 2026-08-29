#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <'text-shadow'> consuming
    // https://drafts.csswg.org/css-text-decor-3/#propdef-text-shadow
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeTextShadow(CSSTokenRange &tokens,
                                                      CSSPropertyParserState &state) noexcept;
  }
}
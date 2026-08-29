#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  struct CSSPropertyParserState;
  class CSSValue;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <single-transition-property-or-none> consuming
    // https://drafts.csswg.org/css-transitions/#single-transition-property
    KRYS_NODISCARD RefPtr<CSSValue>
      ConsumeSingleTransitionPropertyOrNone(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;

    // MARK: <single-transition-property> consuming
    // https://drafts.csswg.org/css-transitions/#single-transition-property
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeSingleTransitionProperty(CSSTokenRange &tokens,
                                                                    CSSPropertyParserState &state) noexcept;
  }
}
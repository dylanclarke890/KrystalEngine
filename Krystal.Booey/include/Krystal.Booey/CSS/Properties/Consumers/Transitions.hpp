#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct PropertyParserState;
  class CSSValue;

  namespace PropertyParserHelpers
  {
    // MARK: <single-transition-property-or-none> consuming
    // https://drafts.csswg.org/css-transitions/#single-transition-property
    KRYS_NODISCARD RefPtr<CSSValue>
      ConsumeSingleTransitionPropertyOrNone(TokenRange &tokens, PropertyParserState &state) noexcept;

    // MARK: <single-transition-property> consuming
    // https://drafts.csswg.org/css-transitions/#single-transition-property
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeSingleTransitionProperty(TokenRange &tokens,
                                                                    PropertyParserState &state) noexcept;
  }
}
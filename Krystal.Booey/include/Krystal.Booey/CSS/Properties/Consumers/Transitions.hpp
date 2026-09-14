#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct PropertyParserState;
  class Value;

  namespace PropertyParserHelpers
  {
    // MARK: <single-transition-property-or-none> consuming
    // https://drafts.csswg.org/css-transitions/#single-transition-property
    KRYS_NODISCARD RefPtr<Value>
      ConsumeSingleTransitionPropertyOrNone(TokenRange &tokens, PropertyParserState &state) noexcept;

    // MARK: <single-transition-property> consuming
    // https://drafts.csswg.org/css-transitions/#single-transition-property
    KRYS_NODISCARD RefPtr<Value> ConsumeSingleTransitionProperty(TokenRange &tokens,
                                                                    PropertyParserState &state) noexcept;
  }
}
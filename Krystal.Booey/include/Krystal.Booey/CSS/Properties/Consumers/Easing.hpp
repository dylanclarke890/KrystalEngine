#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;

  struct PropertyParserState;
  struct EasingFunction;

  namespace PropertyParserHelpers
  {
    // <easing-function> = linear | ease | ease-in | ease-out | ease-in-out | step-start | step-end |
    // <linear()> | <cubic-bezier()> | <steps()> NOTE: also includes non-standard <spring()>.
    // https://drafts.csswg.org/css-easing/#typedef-easing-function

    // MARK: <easing-function> consuming (unresolved)
    KRYS_NODISCARD Maybe<EasingFunction>
      ConsumeUnresolvedEasingFunction(TokenRange &tokens, PropertyParserState &state) noexcept;

    // MARK: <easing-function> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeEasingFunction(TokenRange &tokens,
                                                          PropertyParserState &state) noexcept;
  }
}
#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;

  struct CSSPropertyParserState;
  struct EasingFunction;

  namespace CSSPropertyParserHelpers
  {
    // <easing-function> = linear | ease | ease-in | ease-out | ease-in-out | step-start | step-end |
    // <linear()> | <cubic-bezier()> | <steps()> NOTE: also includes non-standard <spring()>.
    // https://drafts.csswg.org/css-easing/#typedef-easing-function

    // MARK: <easing-function> consuming (unresolved)
    KRYS_NODISCARD Maybe<EasingFunction>
      ConsumeUnresolvedEasingFunction(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;

    // MARK: <easing-function> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeEasingFunction(CSSTokenRange &tokens,
                                                          CSSPropertyParserState &state) noexcept;
  }
}
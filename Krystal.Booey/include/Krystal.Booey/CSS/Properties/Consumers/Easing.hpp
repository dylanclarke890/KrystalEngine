#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class TimingFunction;
}

namespace krys::boo::css
{
  struct ParserContext;
  class TokenRange;
  class Value;

  struct PropertyParserState;
  struct EasingFunction;

  namespace PropertyParserHelpers
  {
    // <easing-function> = linear | ease | ease-in | ease-out | ease-in-out | step-start | step-end |
    // <linear()> | <cubic-bezier()> | <steps()> NOTE: also includes non-standard <spring()>.
    // https://drafts.csswg.org/css-easing/#typedef-easing-function

    // MARK: <easing-function> consuming (unresolved)
    KRYS_NODISCARD Maybe<EasingFunction> ConsumeUnresolvedEasingFunction(TokenRange &tokens,
                                                                         PropertyParserState &state) noexcept;

    // MARK: <easing-function> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeEasingFunction(TokenRange &tokens,
                                                       PropertyParserState &state) noexcept;

    // MARK: <easing-function> parsing (TimingFunction)
    /// @brief Parses the whole of `string`, leading and trailing whitespace aside, as an easing function.
    /// @return Null if it isn't one.
    KRYS_NODISCARD RefPtr<TimingFunction>
      ParseEasingFunctionDeprecated(const CSSOMString &string, const ParserContext &context) noexcept;
  }
}
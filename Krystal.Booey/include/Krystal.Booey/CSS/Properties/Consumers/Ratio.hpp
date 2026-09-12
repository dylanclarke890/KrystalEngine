#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class TokenRange;

  struct CSSPropertyParserState;
  struct Ratio;

  namespace CSSPropertyParserHelpers
  {
    // https://drafts.csswg.org/css-values-4/#ratio-value

    // MARK: <ratio> consuming (unresolved)
    KRYS_NODISCARD Maybe<Ratio> ConsumeUnresolvedRatio(TokenRange &tokens,
                                                       CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD Maybe<Ratio>
      ConsumeUnresolvedRatioWithBothNumeratorAndDenominator(TokenRange &tokens,
                                                            CSSPropertyParserState &state) noexcept;

    // MARK: <ratio> consuming (value)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeRatio(TokenRange &tokens,
                                                 CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue>
      ConsumeRatioWithBothNumeratorAndDenominator(TokenRange &tokens,
                                                  CSSPropertyParserState &state) noexcept;
  }
}
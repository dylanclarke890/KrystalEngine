#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSRatio;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // https://drafts.csswg.org/css-values-4/#ratio-value

    // MARK: <ratio> consuming (unresolved)
    Maybe<CSSRatio> ConsumeUnresolvedRatio(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;
    Maybe<CSSRatio>
      ConsumeUnresolvedRatioWithBothNumeratorAndDenominator(CSSTokenRange &tokens,
                                                            CSSPropertyParserState &state) noexcept;

    // MARK: <ratio> consuming (value)
    RefPtr<CSSValue> ConsumeRatio(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;

    RefPtr<CSSValue> ConsumeRatioWithBothNumeratorAndDenominator(CSSTokenRange &tokens,
                                                                 CSSPropertyParserState &state) noexcept;
  }
}
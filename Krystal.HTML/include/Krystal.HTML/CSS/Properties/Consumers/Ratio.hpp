#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;

  struct CSSPropertyParserState;
  struct Ratio;

  namespace CSSPropertyParserHelpers
  {
    // https://drafts.csswg.org/css-values-4/#ratio-value

    // MARK: <ratio> consuming (unresolved)
    KRYS_NODISCARD Maybe<Ratio> ConsumeUnresolvedRatio(CSSTokenRange &tokens,
                                                       CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD Maybe<Ratio>
      ConsumeUnresolvedRatioWithBothNumeratorAndDenominator(CSSTokenRange &tokens,
                                                            CSSPropertyParserState &state) noexcept;

    // MARK: <ratio> consuming (value)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeRatio(CSSTokenRange &tokens,
                                                 CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue>
      ConsumeRatioWithBothNumeratorAndDenominator(CSSTokenRange &tokens,
                                                  CSSPropertyParserState &state) noexcept;
  }
}
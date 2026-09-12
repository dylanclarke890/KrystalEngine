#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  struct AppleColorFilter;
  class TokenRange;
  class CSSValue;
  struct CSSPropertyParserState;
  struct Filter;

  namespace CSSPropertyParserHelpers
  {
    // https://drafts.fxtf.org/filter-effects/#FilterProperty

    // MARK: <'filter'> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeFilter(TokenRange &tokens,
                                                  CSSPropertyParserState &state) noexcept;

    // MARK: <'-apple-color-filter'> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAppleColorFilter(TokenRange &tokens,
                                                            CSSPropertyParserState &state) noexcept;

    // MARK: <'filter'> consuming (unresolved)
    KRYS_NODISCARD Maybe<Filter> ConsumeUnresolvedFilter(TokenRange &tokens,
                                                         CSSPropertyParserState &state) noexcept;

    // MARK: <'apple-color-filter'> consuming (unresolved)
    KRYS_NODISCARD Maybe<AppleColorFilter>
      ConsumeUnresolvedAppleColorFilter(TokenRange &tokens, CSSPropertyParserState &state) noexcept;

    // MARK: <'filter'> parsing (raw)
    // KRYS_NODISCARD Maybe<StyleFilter> ParseFilterValueListOrNoneRaw(const CSSOMString &, const
    // ParserContext &,
    //                                                                const Document &, RenderStyle &);
  }
}
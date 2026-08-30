#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  struct AppleColorFilter;
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;
  struct Filter;

  namespace CSSPropertyParserHelpers
  {
    // https://drafts.fxtf.org/filter-effects/#FilterProperty

    // MARK: <'filter'> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeFilter(CSSTokenRange &tokens,
                                                  CSSPropertyParserState &state) noexcept;

    // MARK: <'-apple-color-filter'> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAppleColorFilter(CSSTokenRange &tokens,
                                                            CSSPropertyParserState &state) noexcept;

    // MARK: <'filter'> consuming (unresolved)
    KRYS_NODISCARD Maybe<Filter> ConsumeUnresolvedFilter(CSSTokenRange &tokens,
                                                         CSSPropertyParserState &state) noexcept;

    // MARK: <'apple-color-filter'> consuming (unresolved)
    KRYS_NODISCARD Maybe<AppleColorFilter>
      ConsumeUnresolvedAppleColorFilter(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;

    // MARK: <'filter'> parsing (raw)
    // KRYS_NODISCARD Maybe<StyleFilter> ParseFilterValueListOrNoneRaw(const CSSOMString &, const
    // CSSParserContext &,
    //                                                                const Document &, RenderStyle &);
  }
}
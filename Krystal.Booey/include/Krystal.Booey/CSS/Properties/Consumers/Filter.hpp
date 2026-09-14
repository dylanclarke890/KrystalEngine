#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  struct AppleColorFilter;
  class TokenRange;
  class Value;
  struct PropertyParserState;
  struct Filter;

  namespace PropertyParserHelpers
  {
    // https://drafts.fxtf.org/filter-effects/#FilterProperty

    // MARK: <'filter'> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeFilter(TokenRange &tokens,
                                                  PropertyParserState &state) noexcept;

    // MARK: <'-apple-color-filter'> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeAppleColorFilter(TokenRange &tokens,
                                                            PropertyParserState &state) noexcept;

    // MARK: <'filter'> consuming (unresolved)
    KRYS_NODISCARD Maybe<Filter> ConsumeUnresolvedFilter(TokenRange &tokens,
                                                         PropertyParserState &state) noexcept;

    // MARK: <'apple-color-filter'> consuming (unresolved)
    KRYS_NODISCARD Maybe<AppleColorFilter>
      ConsumeUnresolvedAppleColorFilter(TokenRange &tokens, PropertyParserState &state) noexcept;

    // MARK: <'filter'> parsing (raw)
    // KRYS_NODISCARD Maybe<StyleFilter> ParseFilterValueListOrNoneRaw(const CSSOMString &, const
    // ParserContext &,
    //                                                                const Document &, RenderStyle &);
  }
}
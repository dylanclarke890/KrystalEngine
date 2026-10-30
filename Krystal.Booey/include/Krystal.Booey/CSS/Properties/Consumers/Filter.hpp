#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::style
{
  struct Filter;
}

namespace krys::boo::css
{
  struct AppleColorFilter;
  struct ParserContext;
  class TokenRange;
  class Value;
  struct PropertyParserState;
  struct Filter;

  namespace PropertyParserHelpers
  {
    // https://drafts.fxtf.org/filter-effects/#FilterProperty

    // MARK: <'filter'> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeFilter(TokenRange &tokens, PropertyParserState &state) noexcept;

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

    /// @brief @p string parsed as a `filter` value and resolved against @p style; empty if it isn't one.
    /// @note What the canvas 2D context's `filter` is set with.
    KRYS_NODISCARD Maybe<style::Filter> ParseFilterValueListOrNoneRaw(const CSSOMString &string,
                                                                      const ParserContext &context,
                                                                      const dom::Document &document,
                                                                      RenderStyle &style) noexcept;
  }
}
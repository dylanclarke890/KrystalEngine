#pragma once

#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css
{
  // FIXME: Change all call sites to use ParserContext directly and then remove this.
  struct MediaQueryParserContext
  {
    ParserContext Context;

    MediaQueryParserContext() noexcept;

    // Implicit: the media query parsers take a ParserContext where this is expected.
    MediaQueryParserContext(const ParserContext &context) noexcept;

    /// @brief The document's context (`Document::CssParserContext()`).
    explicit MediaQueryParserContext(const dom::Document &document) noexcept;
  };
}

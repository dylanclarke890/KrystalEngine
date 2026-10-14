#pragma once

#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"

namespace krys::boo::css
{
  // FIXME: Change all call sites to use ParserContext directly and then remove this.
  struct MediaQueryParserContext
  {
    ParserContext Context;

    MediaQueryParserContext() noexcept;

    // Implicit: the media query parsers take a ParserContext where this is expected.
    MediaQueryParserContext(const ParserContext &context) noexcept;

    // FIXME: MediaQueryParserContext(const dom::Document &), once Document::CssParserContext() exists.
  };
}

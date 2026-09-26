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
  public:
    MediaQueryParserContext() noexcept;
    
    MediaQueryParserContext(const dom::Document &document) noexcept;

    MediaQueryParserContext(const ParserContext &o) noexcept;

    ParserContext Context;
  };
}

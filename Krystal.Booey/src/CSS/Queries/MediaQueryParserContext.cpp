#include "Krystal.Booey/CSS/Queries/MediaQueryParserContext.hpp"
#include "Krystal.Booey/DOM/Document.hpp"

namespace krys::boo::css
{
  MediaQueryParserContext::MediaQueryParserContext() noexcept : Context(StrictParserContext())
  {
  }

  MediaQueryParserContext::MediaQueryParserContext(const ParserContext &context) noexcept : Context(context)
  {
  }

  MediaQueryParserContext::MediaQueryParserContext(const dom::Document &document) noexcept
      : Context(document.CssParserContext())
  {
  }
}

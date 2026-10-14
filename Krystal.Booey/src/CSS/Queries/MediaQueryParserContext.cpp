#include "Krystal.Booey/CSS/Queries/MediaQueryParserContext.hpp"

namespace krys::boo::css
{
  MediaQueryParserContext::MediaQueryParserContext() noexcept : Context(StrictParserContext())
  {
  }

  MediaQueryParserContext::MediaQueryParserContext(const ParserContext &context) noexcept : Context(context)
  {
  }
}

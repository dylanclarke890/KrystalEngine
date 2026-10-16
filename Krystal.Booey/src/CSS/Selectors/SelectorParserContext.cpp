#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Page/Settings.hpp"

namespace krys::boo::css
{
  SelectorParserContext::SelectorParserContext(const ParserContext &context) noexcept
      : Mode(context.Mode), thumbAndTrackPseudoElementsEnabled(context.thumbAndTrackPseudoElementsEnabled),
        webkitMediaTextTrackDisplayQuirkEnabled(context.webkitMediaTextTrackDisplayQuirkEnabled)
  {
  }

  SelectorParserContext::SelectorParserContext(const dom::Document &document) noexcept
      : Mode(document.InQuirksMode() ? ParserMode::HTMLQuirks : ParserMode::HTMLStandard),
        thumbAndTrackPseudoElementsEnabled(document.GetSettings().ThumbAndTrackPseudoElementsEnabled)
  {
    // FIXME: The media text track display quirk, once documents have quirks.
  }
}

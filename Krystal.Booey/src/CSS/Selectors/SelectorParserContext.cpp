#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/DOM/Document.hpp"

namespace krys::boo::css
{
  SelectorParserContext::SelectorParserContext(const ParserContext &context) noexcept
      : Mode(context.Mode), imageControlsEnabled(context.imageControlsEnabled),
        popoverAttributeEnabled(context.popoverAttributeEnabled),
        targetTextPseudoElementEnabled(context.targetTextPseudoElementEnabled),
        thumbAndTrackPseudoElementsEnabled(context.thumbAndTrackPseudoElementsEnabled),
        viewTransitionsEnabled(context.PropertySettings.viewTransitionsEnabled),
        cssAppearanceBaseEnabled(context.cssAppearanceBaseEnabled),
        webkitMediaTextTrackDisplayQuirkEnabled(context.webkitMediaTextTrackDisplayQuirkEnabled)
  {
  }

  SelectorParserContext::SelectorParserContext(const dom::Document &document) noexcept
      : Mode(document.InQuirksMode() ? ParserMode::HTMLQuirks : ParserMode::HTMLStandard)
  {
  }
}

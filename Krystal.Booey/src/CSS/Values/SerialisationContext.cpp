#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/CSSImportRule.hpp"
#include "Krystal.Booey/CSS/CSSNamespaceRule.hpp"
#include "Krystal.Booey/CSS/CSSRuleList.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/MediaList.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::css
{
  SerialisationContext::SerialisationContext(
    HashMap<CSSOMString, CSSOMString> &&replacementUrlStrings,
    HashMap<Ref<CSSStyleSheet>, CSSOMString> &&replacementUrlStringsForStyleSheet,
    bool shouldUseResolvedURLInCSSText) noexcept
      : ReplacementUrlStrings(krys::move(replacementUrlStrings)),
        ReplacementUrlStringsForStyleSheet(krys::move(replacementUrlStringsForStyleSheet)),
        ShouldUseResolvedUrlInCSSText(shouldUseResolvedURLInCSSText)
  {
  }

  const SerialisationContext &DefaultSerialisationContext() noexcept
  {
    static NeverDestroyed<SerialisationContext> defaultSerialisationContext;
    return defaultSerialisationContext;
  }
}
#include "Krystal.Booey/CSS/CSSOM/CSSStartingStyleRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"

namespace krys::boo::css
{
  CSSStartingStyleRule::CSSStartingStyleRule(StartingStyleRule &rule, CSSStyleSheet *parent) noexcept
      : CSSGroupingRule(rule, parent)
  {
  }

  Ref<CSSStartingStyleRule> CSSStartingStyleRule::Create(StartingStyleRule &rule,
                                                         CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSStartingStyleRule(rule, parent));
  }

  CSSOMString CSSStartingStyleRule::CssText() const noexcept
  {
    CSSOMString builder;
    builder.append(u8"@starting-style");
    AppendCSSTextForItems(builder);
    return builder;
  }
}

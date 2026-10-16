#include "Krystal.Booey/CSS/CSSOM/CSSStartingStyleRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

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
    StringBuilder builder;
    builder.Append(u8"@starting-style");
    AppendCSSTextForItems(builder);
    return krys::move(builder).ToString();
  }
}

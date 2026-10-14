#include "Krystal.Booey/CSS/CSSOM/CSSSupportsRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"

namespace krys::boo::css
{
  CSSSupportsRule::CSSSupportsRule(SupportsRule &rule, CSSStyleSheet *parent) noexcept
      : CSSConditionRule(rule, parent)
  {
  }

  Ref<CSSSupportsRule> CSSSupportsRule::Create(SupportsRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSSupportsRule(rule, parent));
  }

  CSSOMString CSSSupportsRule::CssText() const noexcept
  {
    CSSOMString builder;
    builder.append(u8"@supports ").append(ConditionText());
    AppendCSSTextForItems(builder);
    return builder;
  }

  CSSOMString CSSSupportsRule::CssText(const SerialisationContext &context) const noexcept
  {
    CSSOMString builder;
    builder.append(u8"@supports ").append(ConditionText());
    AppendCSSTextWithReplacementURLsForItems(builder, context);
    return builder;
  }

  CSSOMString CSSSupportsRule::ConditionText() const noexcept
  {
    return Downcast<SupportsRule>(GetGroupRule()).ConditionText();
  }
}

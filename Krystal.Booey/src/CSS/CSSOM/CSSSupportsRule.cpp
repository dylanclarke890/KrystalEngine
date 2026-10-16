#include "Krystal.Booey/CSS/CSSOM/CSSSupportsRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

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
    StringBuilder builder;
    builder.Append(u8"@supports ", ConditionText());
    AppendCSSTextForItems(builder);
    return krys::move(builder).ToString();
  }

  CSSOMString CSSSupportsRule::CssText(const SerialisationContext &context) const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@supports ", ConditionText());
    AppendCSSTextWithReplacementURLsForItems(builder, context);
    return krys::move(builder).ToString();
  }

  CSSOMString CSSSupportsRule::ConditionText() const noexcept
  {
    return Downcast<SupportsRule>(GetGroupRule()).ConditionText();
  }
}

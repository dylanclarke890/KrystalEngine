#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSConditionRule.hpp"

namespace krys::boo::css
{
  class StyleRuleSupports;

  class CSSSupportsRule final : public CSSConditionRule
  {
  public:
    static Ref<CSSSupportsRule> create(StyleRuleSupports &, CSSStyleSheet *parent);

    String cssText() const final;
    String cssText(const SerialisationContext &) const final;
    String conditionText() const final;

  private:
    CSSSupportsRule(StyleRuleSupports &, CSSStyleSheet *);
    StyleRuleType styleRuleType() const final
    {
      return StyleRuleType::Supports;
    }
  };

}

SPECIALIZE_TYPE_TRAITS_CSS_RULE(CSSSupportsRule, StyleRuleType::Supports)

#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSGroupingRule.hpp"

namespace krys::boo::css
{
  class StyleRuleStartingStyle;

  class CSSStartingStyleRule final : public CSSGroupingRule
  {
  public:
    static Ref<CSSStartingStyleRule> create(StyleRuleStartingStyle &rule, CSSStyleSheet *parent)
    {
      return adoptRef(*new CSSStartingStyleRule(rule, parent));
    }

    String cssText() const final;

  private:
    CSSStartingStyleRule(StyleRuleStartingStyle &, CSSStyleSheet *);

    RuleType RuleType() const final
    {
      return RuleType::StartingStyle;
    }
  };

}

SPECIALIZE_TYPE_TRAITS_CSS_RULE(CSSStartingStyleRule, RuleType::StartingStyle)

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

    StyleRuleType styleRuleType() const final
    {
      return StyleRuleType::StartingStyle;
    }
  };

}

SPECIALIZE_TYPE_TRAITS_CSS_RULE(CSSStartingStyleRule, StyleRuleType::StartingStyle)

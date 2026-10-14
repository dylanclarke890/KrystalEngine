#pragma once

#include "Krystal.Booey/CSS/Rules/ViewTransitionRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "StyleProperties.h"

namespace krys::boo::css
{
  class CSSViewTransitionRule final : public CSSRule
  {
  public:
    using ViewTransitionNavigation = WebCore::ViewTransitionNavigation;

    static Ref<CSSViewTransitionRule> create(StyleRuleViewTransition &, CSSStyleSheet *);
    virtual ~CSSViewTransitionRule();

    String cssText() const final;
    void reattach(StyleRuleBase &) final;
    RuleType RuleType() const final
    {
      return RuleType::ViewTransition;
    }

    CSSOMStringAtom navigation() const;
    SmallList<CSSOMStringAtom> types() const
    {
      return Ref {m_viewTransitionRule}->types();
    }

  private:
    CSSViewTransitionRule(StyleRuleViewTransition &, CSSStyleSheet *parent);

    Ref<StyleRuleViewTransition> m_viewTransitionRule;
  };

} // namespace krys::boo::css

SPECIALIZE_TYPE_TRAITS_CSS_RULE(CSSViewTransitionRule, RuleType::ViewTransition)


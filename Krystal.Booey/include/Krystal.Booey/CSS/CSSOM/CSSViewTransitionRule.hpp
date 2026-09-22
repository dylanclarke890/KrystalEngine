#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "StyleProperties.h"
#include "StyleRule.h"
#include <wtf/text/AtomString.h>

namespace krys::boo::css
{
  enum class ViewTransitionNavigation : bool
  {
    Auto,
    None,
  };

  class StyleRuleViewTransition final : public StyleRuleBase
  {
  public:
    static Ref<StyleRuleViewTransition> create(Ref<StyleProperties> &&);
    ~StyleRuleViewTransition();

    Ref<StyleRuleViewTransition> copy() const
    {
      return adoptRef(*new StyleRuleViewTransition(*this));
    }

    std::optional<ViewTransitionNavigation> navigation() const
    {
      return m_navigation;
    }
    ViewTransitionNavigation computedNavigation() const
    {
      return navigation().value_or(ViewTransitionNavigation::None);
    }
    Vector<AtomString> types() const
    {
      return m_types;
    }

  private:
    explicit StyleRuleViewTransition(Ref<StyleProperties> &&);
    StyleRuleViewTransition(const StyleRuleViewTransition &) = default;

    std::optional<ViewTransitionNavigation> m_navigation;
    Vector<AtomString> m_types;
  };

  class CSSViewTransitionRule final : public CSSRule
  {
  public:
    using ViewTransitionNavigation = WebCore::ViewTransitionNavigation;

    static Ref<CSSViewTransitionRule> create(StyleRuleViewTransition &, CSSStyleSheet *);
    virtual ~CSSViewTransitionRule();

    String cssText() const final;
    void reattach(StyleRuleBase &) final;
    StyleRuleType styleRuleType() const final
    {
      return StyleRuleType::ViewTransition;
    }

    AtomString navigation() const;
    Vector<AtomString> types() const
    {
      return Ref {m_viewTransitionRule}->types();
    }

  private:
    CSSViewTransitionRule(StyleRuleViewTransition &, CSSStyleSheet *parent);

    Ref<StyleRuleViewTransition> m_viewTransitionRule;
  };

} // namespace krys::boo::css

SPECIALIZE_TYPE_TRAITS_CSS_RULE(CSSViewTransitionRule, StyleRuleType::ViewTransition)

SPECIALIZE_TYPE_TRAITS_BEGIN(WebCore::StyleRuleViewTransition)
static bool isType(const WebCore::StyleRuleBase &rule)
{
  return rule.isViewTransitionRule();
}
SPECIALIZE_TYPE_TRAITS_END()

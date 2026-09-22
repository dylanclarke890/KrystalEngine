#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSConditionRule.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::css
{
  class CSSRuleList;
  class CSSStyleProperties;
  class DeclaredStylePropertyMap;
  class StylePropertyMap;
  class StyleRuleCSSStyleProperties;
  class StyleRule;
  class StyleRuleWithNesting;
  class StyleRuleCSSStyleProperties;

  class CSSStyleRule final : public CSSRule
  {
  public:
    static Ref<CSSStyleRule> create(StyleRule &rule, CSSStyleSheet *sheet)
    {
      return adoptRef(*new CSSStyleRule(rule, sheet));
    }
    static Ref<CSSStyleRule> create(StyleRuleWithNesting &rule, CSSStyleSheet *sheet)
    {
      return adoptRef(*new CSSStyleRule(rule, sheet));
    };

    virtual ~CSSStyleRule();

    String selectorText() const;
    void setSelectorText(const String &);

    CSSStyleProperties &style();

    // FIXME: Not CSSOM. Remove.
    StyleRule &styleRule() const
    {
      return m_styleRule.get();
    }

    CSSRuleList &cssRules() const;
    ExceptionOr<unsigned> insertRule(const String &rule, unsigned index);
    ExceptionOr<void> deleteRule(unsigned index);
    unsigned length() const;
    CSSRule *item(unsigned index) const;

    StylePropertyMap &styleMap();

  private:
    CSSStyleRule(StyleRule &, CSSStyleSheet *);
    CSSStyleRule(StyleRuleWithNesting &, CSSStyleSheet *);

    StyleRuleType styleRuleType() const final
    {
      return StyleRuleType::Style;
    }
    String cssText() const final;
    String cssText(const SerialisationContext &) const final;
    String cssTextInternal(StringBuilder &declarations, StringBuilder &rules) const;
    void reattach(StyleRuleBase &) final;
    void getChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &) final;

    String generateSelectorText() const;
    Vector<Ref<StyleRuleBase>> nestedRules() const;
    void cssTextForRules(StringBuilder &rules) const;
    void cssTextForRulesWithReplacementURLs(StringBuilder &rules, const SerialisationContext &) const;

    Ref<StyleRule> m_styleRule;
    const Ref<DeclaredStylePropertyMap> m_styleMap;
    RefPtr<StyleRuleCSSStyleProperties> m_propertiesCSSOMWrapper;

    mutable Vector<RefPtr<CSSRule>> m_childRuleCSSOMWrappers;
    const std::unique_ptr<CSSRuleList> m_ruleListCSSOMWrapper;
  };

} // namespace krys::boo::css

SPECIALIZE_TYPE_TRAITS_CSS_RULE(CSSStyleRule, StyleRuleType::Style)

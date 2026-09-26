#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"

namespace krys::boo::css
{

  class CSSFontFeatureValuesRule final : public CSSRule
  {
  public:
    static Ref<CSSFontFeatureValuesRule> create(StyleRuleFontFeatureValues &rule, CSSStyleSheet *sheet)
    {
      return adoptRef(*new CSSFontFeatureValuesRule(rule, sheet));
    }
    virtual ~CSSFontFeatureValuesRule() = default;

    const SmallList<CSSOMStringAtom> &fontFamilies() const
    {
      return m_fontFeatureValuesRule->fontFamilies();
    }

    // Used by the CSSOM.
    CSSOMStringAtom fontFamily() const
    {
      StringBuilder builder;
      bool first = true;
      for (auto &family : m_fontFeatureValuesRule->fontFamilies())
      {
        if (first)
          first = false;
        else
          builder.append(", "_s);

        builder.append(family);
      }
      return builder.toAtomString();
    }

  private:
    CSSFontFeatureValuesRule(StyleRuleFontFeatureValues &, CSSStyleSheet *parent);

    RuleType RuleType() const final
    {
      return RuleType::FontFeatureValues;
    }
    String cssText() const final;
    void reattach(StyleRuleBase &) final;

    Ref<StyleRuleFontFeatureValues> m_fontFeatureValuesRule;
  };

  class CSSFontFeatureValuesBlockRule final : public CSSRule
  {
  public:
    static Ref<CSSFontFeatureValuesBlockRule> create(StyleRuleFontFeatureValuesBlock &rule,
                                                     CSSStyleSheet *sheet)
    {
      return adoptRef(*new CSSFontFeatureValuesBlockRule(rule, sheet));
    }
    virtual ~CSSFontFeatureValuesBlockRule() = default;

  private:
    CSSFontFeatureValuesBlockRule(StyleRuleFontFeatureValuesBlock &, CSSStyleSheet *parent);

    RuleType RuleType() const final
    {
      return RuleType::FontFeatureValuesBlock;
    }
    String cssText() const final;
    void reattach(StyleRuleBase &) final;

    Ref<StyleRuleFontFeatureValuesBlock> m_fontFeatureValuesBlockRule;
  };

} // namespace krys::boo::css

SPECIALIZE_TYPE_TRAITS_CSS_RULE(CSSFontFeatureValuesRule, RuleType::FontFeatureValues)
SPECIALIZE_TYPE_TRAITS_CSS_RULE(CSSFontFeatureValuesBlockRule, RuleType::FontFeatureValuesBlock)

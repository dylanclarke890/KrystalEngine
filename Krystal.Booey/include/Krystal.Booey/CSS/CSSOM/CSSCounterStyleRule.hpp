#pragma once

#include <WebCore/CSSCounterStyle.h>
#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include <WebCore/StyleProperties.h>
#include <WebCore/StyleRule.h>
#include <wtf/text/AtomString.h>

namespace krys::boo::css
{
  class CounterStyleRule final : public RuleBase
  {
  public:
    static Ref<CounterStyleRule> create(const AtomString &, CSSCounterStyleDescriptors &&);
    ~CounterStyleRule();

    Ref<CounterStyleRule> copy() const
    {
      return adoptRef(*new CounterStyleRule(*this));
    }

    const CSSCounterStyleDescriptors &descriptors() const
    {
      return m_descriptors;
    };
    CSSCounterStyleDescriptors &mutableDescriptors()
    {
      return m_descriptors;
    };

    const AtomString &name() const
    {
      return m_name;
    }
    String system() const
    {
      return m_descriptors.systemCSSText();
    }
    String negative() const
    {
      return m_descriptors.negativeCSSText();
    }
    String prefix() const
    {
      return m_descriptors.prefixCSSText();
    }
    String suffix() const
    {
      return m_descriptors.suffixCSSText();
    }
    String range() const
    {
      return {m_descriptors.rangesCSSText()};
    }
    String pad() const
    {
      return m_descriptors.padCSSText();
    }
    String fallback() const
    {
      return m_descriptors.fallbackCSSText();
    }
    String symbols() const
    {
      return m_descriptors.symbolsCSSText();
    }
    String additiveSymbols() const
    {
      return m_descriptors.additiveSymbolsCSSText();
    }
    String speakAs() const
    {
      return {};
    }
    bool newValueInvalidOrEqual(CSSPropertyID, const RefPtr<CSSValue> newValue) const;

    void setName(const AtomString &name)
    {
      m_name = name;
    }

  private:
    explicit StyleRuleCounterStyle(const AtomString &, CSSCounterStyleDescriptors &&);
    StyleRuleCounterStyle(const StyleRuleCounterStyle &) = default;

    AtomString m_name;
    CSSCounterStyleDescriptors m_descriptors;
  };

  class CSSCounterStyleRule final : public CSSRule
  {
  public:
    static Ref<CSSCounterStyleRule> create(StyleRuleCounterStyle &, CSSStyleSheet *);
    virtual ~CSSCounterStyleRule();

    String cssText() const final;
    void reattach(StyleRuleBase &) final;
    StyleRuleType styleRuleType() const final
    {
      return StyleRuleType::CounterStyle;
    }

    String name() const
    {
      return m_counterStyleRule->name();
    }
    String system() const
    {
      return m_counterStyleRule->system();
    }
    String negative() const
    {
      return m_counterStyleRule->negative();
    }
    String prefix() const
    {
      return m_counterStyleRule->prefix();
    }
    String suffix() const
    {
      return m_counterStyleRule->suffix();
    }
    String range() const
    {
      return m_counterStyleRule->range();
    }
    String pad() const
    {
      return m_counterStyleRule->pad();
    }
    String fallback() const
    {
      return m_counterStyleRule->fallback();
    }
    String symbols() const
    {
      return m_counterStyleRule->symbols();
    }
    String additiveSymbols() const
    {
      return m_counterStyleRule->additiveSymbols();
    }
    String speakAs() const
    {
      return m_counterStyleRule->speakAs();
    }

    void setName(const String &);
    void setSystem(const String &);
    void setNegative(const String &);
    void setPrefix(const String &);
    void setSuffix(const String &);
    void setRange(const String &);
    void setPad(const String &);
    void setFallback(const String &);
    void setSymbols(const String &);
    void setAdditiveSymbols(const String &);
    void setSpeakAs(const String &);

  private:
    CSSCounterStyleRule(StyleRuleCounterStyle &, CSSStyleSheet *parent);

    bool setterInternal(CSSPropertyID, const String &);
    RefPtr<CSSValue> cssValueFromText(CSSPropertyID, const String &);
    const CSSCounterStyleDescriptors &descriptors() const
    {
      return m_counterStyleRule->descriptors();
    }
    CSSCounterStyleDescriptors &mutableDescriptors()
    {
      return m_counterStyleRule->mutableDescriptors();
    }

    Ref<StyleRuleCounterStyle> m_counterStyleRule;
  };

  CSSCounterStyleDescriptors::System toCounterStyleSystemEnum(const CSSValue *);
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSCounterStyleRule)
  KRYS_NODISCARD static bool isType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.Type() == krys::boo::css::RuleType::CounterStyle;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::RuleBase)
  KRYS_NODISCARD static bool isType(const krys::boo::css::CounterStyleRule &rule) noexcept
  {
    return rule.IsCounterStyleRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

#pragma once

#include "Krystal.Booey/CSS/Rules/CounterStyleRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include <WebCore/CSSCounterStyle.h>
#include <WebCore/StyleProperties.h>

namespace krys::boo::css
{
  class CSSCounterStyleRule final : public CSSRule
  {
  public:
    virtual ~CSSCounterStyleRule() noexcept;

    KRYS_NODISCARD static Ref<CSSCounterStyleRule> Create(CounterStyleRule &rule,
                                                          CSSStyleSheet *sheet) noexcept;

    String cssText() const final;
    void reattach(StyleRuleBase &) final;
    RuleType RuleType() const final
    {
      return RuleType::CounterStyle;
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
    RefPtr<Value> cssValueFromText(CSSPropertyID, const String &);
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

  CSSCounterStyleDescriptors::System toCounterStyleSystemEnum(const Value *);
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSCounterStyleRule)
  KRYS_NODISCARD static bool isType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.Type() == krys::boo::css::RuleType::CounterStyle;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()


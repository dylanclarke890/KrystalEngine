#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CounterStyleRule;
  class Value;
  struct CounterStyleDescriptors;

  /// @brief An `@counter-style` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-counter-styles/#the-csscounterstylerule-interface
  class CSSCounterStyleRule final : public CSSRule
  {
  private:
    Ref<CounterStyleRule> _counterStyleRule;

    CSSCounterStyleRule(CounterStyleRule &counterStyleRule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSCounterStyleRule() noexcept;

    KRYS_NODISCARD static Ref<CSSCounterStyleRule> Create(CounterStyleRule &rule,
                                                          CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    void Reattach(Rule &rule) noexcept final;

    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::CounterStyle;
    }

    KRYS_NODISCARD CSSOMString Name() const noexcept;
    KRYS_NODISCARD CSSOMString System() const noexcept;
    KRYS_NODISCARD CSSOMString Negative() const noexcept;
    KRYS_NODISCARD CSSOMString Prefix() const noexcept;
    KRYS_NODISCARD CSSOMString Suffix() const noexcept;
    KRYS_NODISCARD CSSOMString Range() const noexcept;
    KRYS_NODISCARD CSSOMString Pad() const noexcept;
    KRYS_NODISCARD CSSOMString Fallback() const noexcept;
    KRYS_NODISCARD CSSOMString Symbols() const noexcept;
    KRYS_NODISCARD CSSOMString AdditiveSymbols() const noexcept;
    KRYS_NODISCARD CSSOMString SpeakAs() const noexcept;

    void SetName(const CSSOMString &text) noexcept;
    void SetSystem(const CSSOMString &text) noexcept;
    void SetNegative(const CSSOMString &text) noexcept;
    void SetPrefix(const CSSOMString &text) noexcept;
    void SetSuffix(const CSSOMString &text) noexcept;
    void SetRange(const CSSOMString &text) noexcept;
    void SetPad(const CSSOMString &text) noexcept;
    void SetFallback(const CSSOMString &text) noexcept;
    void SetSymbols(const CSSOMString &text) noexcept;
    void SetAdditiveSymbols(const CSSOMString &text) noexcept;
    void SetSpeakAs(const CSSOMString &text) noexcept;

  private:
    KRYS_NODISCARD RefPtr<Value> CSSValueFromText(PropertyId property,
                                                  const CSSOMString &text) const noexcept;

    KRYS_NODISCARD const CounterStyleDescriptors &Descriptors() const noexcept;
    KRYS_NODISCARD CounterStyleDescriptors &MutableDescriptors() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSCounterStyleRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::CounterStyle;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

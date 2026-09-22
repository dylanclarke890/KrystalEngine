#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSGroupingRule.hpp"

namespace krys::boo::css
{
  class RuleScope;

  class CSSScopeRule final : public CSSGroupingRule
  {
    CSSScopeRule(RuleScope &scope, CSSStyleSheet *parent) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSScopeRule> Create(RuleScope &scope, CSSStyleSheet *parent) noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

    KRYS_NODISCARD CSSOMString Start() const noexcept;

    KRYS_NODISCARD CSSOMString End() const noexcept;

  private:
    KRYS_NODISCARD const RuleScope &Scope() const noexcept;

    KRYS_NODISCARD RuleType Type() const noexcept final
    {
      return RuleType::Scope;
    }
  };
}

SPECIALIZE_TYPE_TRAITS_CSS_RULE(CSSScopeRule, StyleRuleType::Scope)

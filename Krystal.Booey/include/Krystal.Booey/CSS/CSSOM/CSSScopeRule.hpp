#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSGroupingRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css
{
  class ScopeRule;

  /// @brief An `@scope` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-cascade-6/#the-cssscoperule-interface
  class CSSScopeRule final : public CSSGroupingRule
  {
  private:
    CSSScopeRule(css::ScopeRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSScopeRule> Create(css::ScopeRule &rule, CSSStyleSheet *parent) noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    KRYS_NODISCARD CSSOMString Start() const noexcept;
    KRYS_NODISCARD CSSOMString End() const noexcept;

  private:
    KRYS_NODISCARD const css::ScopeRule &GetScopeRule() const noexcept;

    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::Scope;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSScopeRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Scope;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

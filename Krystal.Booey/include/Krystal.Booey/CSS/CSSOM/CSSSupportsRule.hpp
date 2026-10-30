#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSConditionRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css
{
  class SupportsRule;

  /// @brief An `@supports` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-conditional/#the-csssupportsrule-interface
  class CSSSupportsRule final : public CSSConditionRule
  {
  private:
    CSSSupportsRule(SupportsRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSSupportsRule> Create(SupportsRule &rule, CSSStyleSheet *parent) noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    KRYS_NODISCARD CSSOMString CssText(const SerialisationContext &context) const noexcept final;
    KRYS_NODISCARD CSSOMString ConditionText() const noexcept final;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::Supports;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSSupportsRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Supports;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

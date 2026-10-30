#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSGroupingRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css
{
  class StartingStyleRule;

  /// @brief An `@starting-style` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-transitions-2/#the-cssstartingstylerule-interface
  class CSSStartingStyleRule final : public CSSGroupingRule
  {
  private:
    CSSStartingStyleRule(StartingStyleRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSStartingStyleRule> Create(StartingStyleRule &rule,
                                                           CSSStyleSheet *parent) noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::StartingStyle;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSStartingStyleRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::StartingStyle;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSConditionRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css
{
  class ContainerRule;

  /// @brief An `@container` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-conditional-5/#the-csscontainerrule-interface
  class CSSContainerRule final : public CSSConditionRule
  {
  private:
    CSSContainerRule(css::ContainerRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSContainerRule> Create(css::ContainerRule &rule,
                                                       CSSStyleSheet *parent) noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    KRYS_NODISCARD CSSOMString ConditionText() const noexcept final;
    KRYS_NODISCARD CSSOMString ContainerName() const noexcept;
    KRYS_NODISCARD CSSOMString ContainerQuery() const noexcept;

  private:
    KRYS_NODISCARD const css::ContainerRule &GetContainerRule() const noexcept;

    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::Container;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSContainerRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Container;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

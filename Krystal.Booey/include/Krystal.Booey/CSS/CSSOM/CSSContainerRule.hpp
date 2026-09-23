#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSConditionRule.hpp"

namespace krys::boo::css
{
  class ContainerRule;

  class CSSContainerRule final : public CSSConditionRule
  {
  private:
    CSSContainerRule(ContainerRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSContainerRule> Create(ContainerRule &rule, CSSStyleSheet *parent) noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

    KRYS_NODISCARD CSSOMString ConditionText() const noexcept final;

    KRYS_NODISCARD CSSOMString ContainerName() const noexcept;

    KRYS_NODISCARD CSSOMString ContainerQuery() const noexcept;

  private:
    KRYS_NODISCARD RuleType Type() const noexcept final
    {
      return RuleType::Container;
    }

    const ContainerRule &ContainerRule() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSContainerRule)
  KRYS_NODISCARD static bool isType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.Type() == krys::boo::css::RuleType::Container;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

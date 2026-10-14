#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSGroupingRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css
{
  /// @brief The base of the grouping rules with a condition (`@media`, `@supports`, `@container`).
  /// @see https://drafts.csswg.org/css-conditional/#the-cssconditionrule-interface
  class CSSConditionRule : public CSSGroupingRule
  {
  protected:
    CSSConditionRule(css::GroupRule &groupRule, CSSStyleSheet *parent) noexcept;

  public:
    KRYS_NODISCARD virtual CSSOMString ConditionText() const noexcept = 0;

    KRYS_NODISCARD bool IsCSSConditionRule() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSConditionRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSGroupingRule &rule) noexcept
  {
    return rule.IsCSSConditionRule();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.IsGroupingRule()
           && static_cast<const krys::boo::css::CSSGroupingRule &>(rule).IsCSSConditionRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSGroupingRule.hpp"

namespace krys::boo::css
{
  class CSSConditionRule : public CSSGroupingRule
  {
  protected:
    CSSConditionRule(GroupRule &group, CSSStyleSheet *parent) noexcept;

  public:
    virtual CSSOMString ConditionText() const = 0;

    bool IsCSSConditionRule() const final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_BEGIN(krys::boo::css::CSSConditionRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSGroupingRule &rule) noexcept
  {
    return rule.IsCSSConditionRule();
  }
KRYS_SPECIALIZE_TYPE_TRAITS_END()

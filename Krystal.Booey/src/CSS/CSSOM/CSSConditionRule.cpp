#include "Krystal.Booey/CSS/CSSOM/CSSConditionRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"

namespace krys::boo::css
{
  CSSConditionRule::CSSConditionRule(css::GroupRule &groupRule, CSSStyleSheet *parent) noexcept
      : CSSGroupingRule(groupRule, parent)
  {
  }
}

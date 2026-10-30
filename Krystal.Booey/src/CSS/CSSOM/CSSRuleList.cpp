#include "Krystal.Booey/CSS/CSSOM/CSSRuleList.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"

namespace krys::boo::css
{
  CSSRuleList::CSSRuleList() noexcept = default;

  CSSRuleList::~CSSRuleList() noexcept = default;

  StaticCSSRuleList::StaticCSSRuleList() noexcept = default;

  StaticCSSRuleList::~StaticCSSRuleList() noexcept = default;

  Ref<StaticCSSRuleList> StaticCSSRuleList::Create() noexcept
  {
    return AdoptRef(*new StaticCSSRuleList());
  }
}

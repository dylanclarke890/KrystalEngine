#include "Krystal.Booey/Css/CSSOM/CSSRuleList.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSRule.hpp"

namespace krys::boo
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

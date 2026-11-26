#include "Krystal.Web/Css/CSSOM/CSSRuleList.hpp"
#include "Krystal.Web/Css/CSSOM/CSSRule.hpp"

namespace krys::web
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

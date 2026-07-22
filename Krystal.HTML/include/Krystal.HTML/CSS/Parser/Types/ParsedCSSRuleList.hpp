#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  class CSSRule;

  using ParsedCSSRuleList = SmallList<Ref<CSSRule>>;
}
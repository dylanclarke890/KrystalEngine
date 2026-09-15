#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  class CSSRule;

  using ParsedRuleList = SmallList<Ref<CSSRule>>;
}
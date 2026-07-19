#pragma once

#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSRuleList : public RefCounted<CSSRuleList>
  {
  };
}
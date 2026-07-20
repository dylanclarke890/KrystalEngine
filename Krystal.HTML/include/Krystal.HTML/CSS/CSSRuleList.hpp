#pragma once

#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  /// @see https://drafts.csswg.org/cssom/#the-cssrulelist-interface
  class CSSRuleList : public RefCounted<CSSRuleList>
  {
  public:
#pragma region CSSRuleList - https://drafts.csswg.org/cssom/#cssrulelist
#pragma endregion
  };
}
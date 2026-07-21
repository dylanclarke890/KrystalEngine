#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class CSSRule;

  struct BlockContents
  {
    List<Ref<CSSRule>> Rules;
  };
}
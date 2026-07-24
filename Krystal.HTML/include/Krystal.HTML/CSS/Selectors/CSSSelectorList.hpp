#pragma once

#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"
#include "Krystal.HTML/CSS/Selectors/CSSSelector.hpp"

namespace Krys::HTML
{
  class CSSSelectorList
  {
  private:
    SmallList<UniquePtr<CSSSelector>> _selectors;
  };
}
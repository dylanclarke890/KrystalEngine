#pragma once

#include "Krystal.Booey/CSS/Parser/ParserMode.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/IsCaseSensitive.hpp"

namespace krys::boo::css
{
  struct SelectorParserContext
  {
    ParserMode Mode;
    IsCaseSensitive DefaultCaseSensitivity;
  };
}
#pragma once

#include "Krystal.Booey/CSS/Parser/Context/ParserMode.hpp"
#include "Krystal.Booey/CSS/Selectors/Types/IsCaseSensitive.hpp"

namespace krys::boo::css
{
  struct CSSSelectorParserContext
  {
    ParserMode Mode;
    IsCaseSensitive DefaultCaseSensitivity;
  };
}
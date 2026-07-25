#pragma once

#include "Krystal.HTML/CSS/Parser/Enums/CSSParserMode.hpp"
#include "Krystal.HTML/CSS/Selectors/Types/IsCaseSensitive.hpp"

namespace Krys::HTML
{
  struct CSSSelectorParserContext
  {
    CSSParserMode Mode;
    IsCaseSensitive DefaultCaseSensitivity;
  };
}
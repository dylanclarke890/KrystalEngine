#pragma once

#include "Krystal.HTML/CSS/Parser/Enums/CSSParserMode.hpp"
#include "Krystal.HTML/CSS/Properties/Enums/CSSPropertyId.hpp"

namespace Krys::HTML
{
  struct CSSParserContext
  {
    CSSParserMode Mode;
    CSSPropertySettings PropertySettings;
  };
}
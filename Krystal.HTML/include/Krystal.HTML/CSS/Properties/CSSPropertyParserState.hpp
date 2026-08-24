#pragma once

#include "Krystal.HTML/CSS/Enums/CSSRuleType.hpp"
#include "Krystal.HTML/CSS/Parser/CSSParserContext.hpp"
#include "Krystal.HTML/CSS/Properties/Enums/CSSPropertyId.hpp"
#include "Krystal.HTML/CSS/Properties/Types/IsImportant.hpp"

namespace Krys::HTML
{
  struct CSSPropertyParserState
  {
    const CSSParserContext &Context;
    CSSPropertyId CurrentProperty;
    CSSRuleType CurrentRule;
    IsImportant Important;
  };
}
#pragma once

#include "Krystal.HTML/CSS/Parser/Types/ParsedCSSPropertyList.hpp"
#include "Krystal.HTML/CSS/Parser/Types/ParsedCSSRuleList.hpp"

namespace Krys::HTML
{
  struct NestedParsingContext
  {
    ParsedCSSRuleList ParsedRules;
    ParsedCSSPropertyList ParsedProperties;
  };
}
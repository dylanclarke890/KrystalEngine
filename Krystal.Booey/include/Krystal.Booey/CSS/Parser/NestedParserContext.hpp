#pragma once

#include "Krystal.Booey/CSS/Parser/ParserEnum.hpp"
#include "Krystal.Booey/CSS/Properties/ParsedPropertyList.hpp"
#include "Krystal.Booey/CSS/Rules/ParsedRuleList.hpp"

namespace krys::boo::css
{
  struct NestedParserContext
  {
    ParsedRuleList ParsedRules;
    ParsedPropertyList ParsedProperties;
  };
}

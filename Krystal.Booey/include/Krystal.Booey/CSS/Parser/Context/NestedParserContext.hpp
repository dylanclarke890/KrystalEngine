#pragma once

#include "Krystal.Booey/CSS/Parser/Context/ParsedPropertyList.hpp"
#include "Krystal.Booey/CSS/Parser/Context/ParsedRuleList.hpp"
#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  enum class NestedParserContextType : uint8
  {
    Style,
    Scope,
    Function,
  };

  struct NestedParserContext
  {
    ParsedRuleList ParsedRules;
    ParsedPropertyList ParsedProperties;
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::NestedParserContextType, 3uz);

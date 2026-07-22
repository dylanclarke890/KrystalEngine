#pragma once
#include "Krystal.HTML/CSS/Enums/CSSRuleType.hpp"
#include "Krystal.HTML/CSS/Parser/CSSToken.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Parser/Types/ParsedCSSPropertyList.hpp"
#include "Krystal.HTML/CSS/Properties/CSSProperty.hpp"
#include "Krystal.HTML/CSS/Properties/Types/IsImportant.hpp"

namespace Krys::HTML
{
  class CSSPropertyParser
  {
  public:
    static bool ParseValue(CSSTokenRange tokens, CSSPropertyId id, CSSRuleType ruleType,
                           IsImportant important, ParsedCSSPropertyList &properties) noexcept;
  };
}
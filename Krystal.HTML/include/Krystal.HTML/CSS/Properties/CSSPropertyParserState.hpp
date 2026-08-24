#pragma once

#include "Krystal.HTML/CSS/Enums/CSSRuleType.hpp"
#include "Krystal.HTML/CSS/Parser/CSSParserContext.hpp"
#include "Krystal.HTML/CSS/Properties/Enums/CSSPropertyId.hpp"
#include "Krystal.HTML/CSS/Values/CSSValuePool.hpp"
#include "Krystal.HTML/CSS/Properties/Types/IsImportant.hpp"

namespace Krys::HTML
{
  struct CSSPropertyParserState
  {
    const CSSParserContext &Context;
    CSSValuePool &ValuePool {CSSValuePool::MainThreadPool()};
    CSSPropertyId CurrentProperty;
    CSSRuleType CurrentRule;
    IsImportant Important;
  };
}
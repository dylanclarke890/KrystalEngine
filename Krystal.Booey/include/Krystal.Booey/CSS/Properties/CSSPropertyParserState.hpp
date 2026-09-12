#pragma once

#include "Krystal.Booey/CSS/Parser/Context/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/CSS/Values/CSSValuePool.hpp"

namespace krys::boo::css
{
  struct CSSPropertyParserState
  {
    const ParserContext &Context;
    CSSValuePool &ValuePool {CSSValuePool::MainThreadPool()};
    CSSPropertyId CurrentProperty;
    RuleType CurrentRule;
    IsImportant Important;
  };
}
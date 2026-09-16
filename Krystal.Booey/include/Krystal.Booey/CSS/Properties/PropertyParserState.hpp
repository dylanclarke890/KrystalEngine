#pragma once

#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"

namespace krys::boo::css
{
  struct PropertyParserState
  {
    const ParserContext &Context;
    ValuePool &ValuePool {ValuePool::MainThreadPool()};
    PropertyId CurrentProperty;
    RuleType CurrentRule;
    IsImportant Important;
  };
}
#pragma once

#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::css
{
  class ValuePool;

  struct PropertyParserState
  {
    const ParserContext &Context;

    // TODO: make this `ValuePool &` defaulting to `ValuePool::MainThreadPool()` once `ValuePool` builds.
    ValuePool *Pool {nullptr};

    RuleType CurrentRule {RuleType::Style};
    PropertyId CurrentProperty {PropertyId::Invalid};
    IsImportant Important {false};

    /// @brief Count of CSS random() functions seen so far for the current property.
    uint32 CSSRandomFunctionCount {0};
  };
}

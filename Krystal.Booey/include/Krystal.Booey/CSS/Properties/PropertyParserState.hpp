#pragma once

#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::css
{
  struct PropertyParserState
  {
    const ParserContext &Context;

    ValuePool &Pool {ValuePool::MainThreadPool()};

    RuleType CurrentRule {RuleType::Style};
    PropertyId CurrentProperty {PropertyId::Invalid};
    IsImportant Important {false};

    /// @brief Count of CSS random() functions seen so far for the current property.
    uint32 CSSRandomFunctionCount {0};
  };
}

#pragma once

#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace Krys::HTML
{
  template <CSSValueId Id>
  struct ConsumerDefinition<ValueKeywordConstant<Id>>
  {
    using IdentToken = KeywordConsumer<ValueKeywordConstant<Id>>;
  };
}
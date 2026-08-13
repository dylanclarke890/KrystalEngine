#pragma once

#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace Krys::HTML
{
  template <CSSValueId Id>
  struct ConsumerDefinition<KeywordConstant<Id>>
  {
    using IdentToken = KeywordConsumer<KeywordConstant<Id>>;
  };
}
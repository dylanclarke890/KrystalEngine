#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace krys::boo::css
{
  template <CSSValueId Id>
  struct ConsumerDefinition<ValueKeywordConstant<Id>>
  {
    using IdentToken = KeywordConsumer<ValueKeywordConstant<Id>>;
  };
}
#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace krys::boo::css
{
  template <ValueId Id>
  struct ConsumerDefinition<KeywordValueConstant<Id>>
  {
    using IdentToken = KeywordConsumer<KeywordValueConstant<Id>>;
  };
}
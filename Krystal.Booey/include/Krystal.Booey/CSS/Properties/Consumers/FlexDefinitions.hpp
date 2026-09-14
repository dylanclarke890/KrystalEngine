#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace krys::boo::css
{
  struct FlexValidator
  {
    KRYS_NODISCARD constexpr static Maybe<FlexUnit> Validate(UnitType unitType, PropertyParserState &,
                                                             PropertyParserOptions) noexcept
    {
      return UnitTraits<FlexUnit>::Validate(unitType);
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(FlexRaw<R, V> raw, PropertyParserOptions) noexcept
    {
      return IsValidCanonicalValue(raw);
    }
  };

  template <Range R, typename V>
  struct ConsumerDefinition<Flex<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<Flex<R, V>>;
    using DimensionToken = DimensionConsumer<Flex<R, V>, FlexValidator>;
  };
}
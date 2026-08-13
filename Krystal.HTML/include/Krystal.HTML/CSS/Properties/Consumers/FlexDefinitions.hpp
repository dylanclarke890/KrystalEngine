#pragma once

#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace Krys::HTML
{
  struct FlexValidator
  {
    KRYS_NODISCARD constexpr static Maybe<FlexUnit> Validate(CSSUnitType unitType, CSSPropertyParserState &,
                                                             CSSPropertyParserOptions) noexcept
    {
      return UnitTraits<FlexUnit>::Validate(unitType);
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(FlexRaw<R, V> raw, CSSPropertyParserOptions) noexcept
    {
      return IsValidCanonicalValue(raw);
    }
  };

  template <CSSRange R, typename V>
  struct ConsumerDefinition<Flex<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<Flex<R, V>>;
    using DimensionToken = DimensionConsumer<Flex<R, V>, FlexValidator>;
  };
}
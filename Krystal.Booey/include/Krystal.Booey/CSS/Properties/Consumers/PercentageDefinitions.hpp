#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace krys::boo::css
{
  struct PercentageValidator
  {
    KRYS_NODISCARD constexpr static Maybe<PercentageUnit>
      Validate(UnitType unitType, PropertyParserState &, PropertyParserOptions) noexcept
    {
      return UnitTraits<PercentageUnit>::Validate(unitType);
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(PercentageRaw<R, V> raw, PropertyParserOptions) noexcept
    {
      return IsValidCanonicalValue(raw);
    }
  };

  template <Range R, typename V>
  struct ConsumerDefinition<Percentage<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<Percentage<R, V>>;
    using PercentageToken = PercentageConsumer<Percentage<R, V>, PercentageValidator, PercentageUnit::Percentage>;
  };
}
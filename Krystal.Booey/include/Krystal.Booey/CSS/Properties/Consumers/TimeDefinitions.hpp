#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesCanonicalization.hpp"

namespace krys::boo::css
{
  struct TimeValidator
  {
    KRYS_NODISCARD constexpr static Maybe<TimeUnit> Validate(UnitType unitType, PropertyParserState &,
                                                             PropertyParserOptions) noexcept
    {
      return UnitTraits<TimeUnit>::Validate(unitType);
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(TimeRaw<R, V> raw, PropertyParserOptions) noexcept
    {
      return IsValidDimensionValue(raw,
                                   [&]
                                   {
                                     auto canonicalValue = Canonicalize(raw);
                                     return canonicalValue >= raw.Range.Min
                                            && canonicalValue <= raw.Range.Max;
                                   });
    }
  };

  template <Range R, typename V>
  struct ConsumerDefinition<Time<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<Time<R, V>>;
    using DimensionToken = DimensionConsumer<Time<R, V>, TimeValidator>;
  };
}
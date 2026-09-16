#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesCanonicalization.hpp"

namespace krys::boo::css
{
  struct ResolutionValidator
  {
    KRYS_NODISCARD constexpr static Maybe<ResolutionUnit> Validate(UnitType unitType, PropertyParserState &,
                                                                   PropertyParserOptions) noexcept
    {
      return UnitTraits<ResolutionUnit>::Validate(unitType);
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(ResolutionRaw<R, V> raw, PropertyParserOptions) noexcept
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
  struct ConsumerDefinition<Resolution<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<Resolution<R, V>>;
    using DimensionToken = DimensionConsumer<Resolution<R, V>, ResolutionValidator>;
  };
}
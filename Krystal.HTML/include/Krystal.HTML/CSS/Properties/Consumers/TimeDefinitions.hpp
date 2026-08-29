#pragma once

#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypesCanonicalization.hpp"

namespace Krys::HTML
{
  struct TimeValidator
  {
    KRYS_NODISCARD constexpr static Maybe<TimeUnit> Validate(CSSUnitType unitType, CSSPropertyParserState &,
                                                             CSSPropertyParserOptions) noexcept
    {
      return UnitTraits<TimeUnit>::Validate(unitType);
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(TimeRaw<R, V> raw, CSSPropertyParserOptions) noexcept
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

  template <CSSRange R, typename V>
  struct ConsumerDefinition<Time<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<Time<R, V>>;
    using DimensionToken = DimensionConsumer<Time<R, V>, TimeValidator>;
  };
}
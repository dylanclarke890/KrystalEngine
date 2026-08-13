#pragma once

#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypesCanonicalization.hpp"

namespace Krys::HTML
{
  struct FrequencyValidator
  {
    KRYS_NODISCARD constexpr static Maybe<FrequencyUnit>
      Validate(CSSUnitType unitType, CSSPropertyParserState &, CSSPropertyParserOptions) noexcept
    {
      return UnitTraits<FrequencyUnit>::Validate(unitType);
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(FrequencyRaw<R, V> raw, CSSPropertyParserOptions) noexcept
    {
      return IsValidDimensionValue(raw,
                                   [&]
                                   {
                                     auto canonicalValue = Canonicalize(raw);
                                     return canonicalValue >= raw.range.min
                                            && canonicalValue <= raw.range.max;
                                   });
    }
  };

  template <CSSRange R, typename V>
  struct ConsumerDefinition<Frequency<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<Frequency<R, V>>;
    using DimensionToken = DimensionConsumer<Frequency<R, V>, FrequencyValidator>;
  };
}
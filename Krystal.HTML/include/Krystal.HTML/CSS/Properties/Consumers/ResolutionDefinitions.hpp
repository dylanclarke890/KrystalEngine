#pragma once

#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypesCanonicalization.hpp"

namespace Krys::HTML
{
  struct ResolutionValidator
  {
    KRYS_NODISCARD constexpr static Maybe<ResolutionUnit>
      Validate(CSSUnitType unitType, CSSPropertyParserState &, CSSPropertyParserOptions) noexcept
    {
      return UnitTraits<ResolutionUnit>::Validate(unitType);
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(ResolutionRaw<R, V> raw, CSSPropertyParserOptions) noexcept
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
  struct ConsumerDefinition<Resolution<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<Resolution<R, V>>;
    using DimensionToken = DimensionConsumer<Resolution<R, V>, ResolutionValidator>;
  };
}
#pragma once

#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace Krys::HTML
{
  struct PercentageValidator
  {
    KRYS_NODISCARD constexpr static Maybe<PercentageUnit>
      Validate(CSSUnitType unitType, CSSPropertyParserState &, CSSPropertyParserOptions) noexcept
    {
      return UnitTraits<PercentageUnit>::Validate(unitType);
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(PercentageRaw<R, V> raw, CSSPropertyParserOptions) noexcept
    {
      return IsValidCanonicalValue(raw);
    }
  };

  template <CSSRange R, typename V>
  struct ConsumerDefinition<Percentage<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<Percentage<R, V>>;
    using PercentageToken = PercentageConsumer<Percentage<R, V>, PercentageValidator>;
  };
}
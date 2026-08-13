#pragma once

#include "Krystal.HTML/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace Krys::HTML
{
  struct AnglePercentageValidator
  {
    KRYS_NODISCARD constexpr static Maybe<AnglePercentageUnit>
      Validate(CSSUnitType unitType, CSSPropertyParserState &state, CSSPropertyParserOptions options) noexcept
    {
      // NOTE: Percentages are handled explicitly by the PercentageValidator, so this only
      // needs to be concerned with the Angle units.
      if (auto result = AngleValidator::Validate(unitType, state, options))
      {
        return static_cast<AnglePercentageUnit>(*result);
      }

      return Null;
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(AnglePercentageRaw<R, V> raw, CSSPropertyParserOptions) noexcept
    {
      // Values other than 0 and +/-∞ are not supported for <angle-percentage> numeric ranges currently.
      return IsValidNonCanonicalizableDimensionValue(raw);
    }

    KRYS_NODISCARD static bool ShouldAcceptUnitlessValue(double value, CSSPropertyParserState &state,
                                                         CSSPropertyParserOptions options) noexcept
    {
      return AngleValidator::ShouldAcceptUnitlessValue(value, state, options);
    }
  };

  template <CSSRange R, typename V>
  struct ConsumerDefinition<AnglePercentage<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<AnglePercentage<R, V>>;
    using DimensionToken = DimensionConsumer<AnglePercentage<R, V>, AnglePercentageValidator>;
    using PercentageToken = PercentageConsumer<AnglePercentage<R, V>, AnglePercentageValidator>;
    using NumberToken =
      NumberConsumerForUnitlessValues<AnglePercentage<R, V>, AnglePercentageValidator, AngleUnit::Deg>;
  };
}
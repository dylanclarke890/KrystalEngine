#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace krys::boo::css
{
  struct AnglePercentageValidator
  {
    KRYS_NODISCARD constexpr static Maybe<AnglePercentageUnit>
      Validate(UnitType unitType, PropertyParserState &state, PropertyParserOptions options) noexcept
    {
      // NOTE: Percentages are handled explicitly by the PercentageValidator, so this only
      // needs to be concerned with the Angle units.
      if (auto result = AngleValidator::Validate(unitType, state, options))
      {
        return static_cast<AnglePercentageUnit>(*result);
      }

      return null;
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(AnglePercentageRaw<R, V> raw, PropertyParserOptions) noexcept
    {
      // Values other than 0 and +/-∞ are not supported for <angle-percentage> numeric ranges currently.
      return IsValidNonCanonicalizableDimensionValue(raw);
    }

    KRYS_NODISCARD static bool ShouldAcceptUnitlessValue(double value, PropertyParserState &state,
                                                         PropertyParserOptions options) noexcept
    {
      return AngleValidator::ShouldAcceptUnitlessValue(value, state, options);
    }
  };

  template <Range R, typename V>
  struct ConsumerDefinition<AnglePercentage<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<AnglePercentage<R, V>>;
    using DimensionToken = DimensionConsumer<AnglePercentage<R, V>, AnglePercentageValidator>;
    using PercentageToken = PercentageConsumer<AnglePercentage<R, V>, AnglePercentageValidator, AnglePercentageUnit::Percentage>;
    using NumberToken =
      NumberConsumerForUnitlessValues<AnglePercentage<R, V>, AnglePercentageValidator, AngleUnit::Deg>;
  };
}
#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace krys::boo::css
{
  struct LengthPercentageValidator
  {
    KRYS_NODISCARD constexpr static Maybe<LengthPercentageUnit>
      Validate(UnitType unitType, PropertyParserState &state, PropertyParserOptions options) noexcept
    {
      // NOTE: Percentages are handled explicitly by the PercentageValidator, so this only
      // needs to be concerned with the Length units.
      if (auto result = LengthValidator::Validate(unitType, state, options))
      {
        return static_cast<LengthPercentageUnit>(*result);
      }

      return null;
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(LengthPercentageRaw<R, V> raw, PropertyParserOptions) noexcept
    {
      // Values other than 0 and +/-∞ are not supported for <length-percentage> numeric ranges currently.
      return IsValidNonCanonicalizableDimensionValue(raw);
    }

    KRYS_NODISCARD static bool ShouldAcceptUnitlessValue(double value, PropertyParserState &state,
                                                         PropertyParserOptions options) noexcept
    {
      return LengthValidator::ShouldAcceptUnitlessValue(value, state, options);
    }
  };

  template <Range R, typename V>
  struct ConsumerDefinition<LengthPercentage<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<LengthPercentage<R, V>>;
    using DimensionToken = DimensionConsumer<LengthPercentage<R, V>, LengthPercentageValidator>;
    using PercentageToken = PercentageConsumer<LengthPercentage<R, V>, LengthPercentageValidator, LengthPercentageUnit::Percentage>;
    using NumberToken = NumberConsumerForUnitlessValues<LengthPercentage<R, V>, LengthPercentageValidator,
                                                        LengthPercentageUnit::Px>;
  };
}
#pragma once

#include "Krystal.HTML/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace Krys::HTML
{
  struct LengthPercentageValidator
  {
    KRYS_NODISCARD constexpr static Maybe<LengthPercentageUnit>
      Validate(CSSUnitType unitType, CSSPropertyParserState &state, CSSPropertyParserOptions options) noexcept
    {
      // NOTE: Percentages are handled explicitly by the PercentageValidator, so this only
      // needs to be concerned with the Length units.
      if (auto result = LengthValidator::Validate(unitType, state, options))
      {
        return static_cast<LengthPercentageUnit>(*result);
      }

      return Null;
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(LengthPercentageRaw<R, V> raw, CSSPropertyParserOptions) noexcept
    {
      // Values other than 0 and +/-∞ are not supported for <length-percentage> numeric ranges currently.
      return IsValidNonCanonicalizableDimensionValue(raw);
    }

    KRYS_NODISCARD static bool ShouldAcceptUnitlessValue(double value, CSSPropertyParserState &state,
                                                         CSSPropertyParserOptions options) noexcept
    {
      return LengthValidator::ShouldAcceptUnitlessValue(value, state, options);
    }
  };

  template <CSSRange R, typename V>
  struct ConsumerDefinition<LengthPercentage<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<LengthPercentage<R, V>>;
    using DimensionToken = DimensionConsumer<LengthPercentage<R, V>, LengthPercentageValidator>;
    using PercentageToken = PercentageConsumer<LengthPercentage<R, V>, LengthPercentageValidator, LengthPercentageUnit::Percentage>;
    using NumberToken = NumberConsumerForUnitlessValues<LengthPercentage<R, V>, LengthPercentageValidator,
                                                        LengthPercentageUnit::Px>;
  };
}
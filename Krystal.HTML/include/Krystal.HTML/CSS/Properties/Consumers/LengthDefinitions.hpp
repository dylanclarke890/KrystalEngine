#pragma once

#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace Krys::HTML
{
  struct LengthValidator
  {
    KRYS_NODISCARD constexpr static Maybe<LengthUnit>
      Validate(CSSUnitType unitType, CSSPropertyParserState &state, CSSPropertyParserOptions options) noexcept
    {
      return UnitTraits<LengthUnit>::Validate(unitType);
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(LengthRaw<R, V> raw, CSSPropertyParserOptions) noexcept
    {
      // Values other than 0 and +/-∞ are not supported for <length> numeric ranges currently.
      return IsValidNonCanonicalizableDimensionValue(raw);
    }

    KRYS_NODISCARD static bool ShouldAcceptUnitlessValue(double value, CSSPropertyParserState &state,
                                                         CSSPropertyParserOptions options) noexcept
    {
      if (!value && options.UnitlessZeroLength)
      {
        return true;
      }

      auto mode = options.OverrideParserMode.value_or(state.Context.Mode);
      if (IsUnitlessValueParsingForcedForMode(mode))
      {
        return true;
      }

      return false;
    }
  };

  template <CSSRange R, typename V>
  struct ConsumerDefinition<Length<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<Length<R, V>>;
    using DimensionToken = DimensionConsumer<Length<R, V>, LengthValidator>;
    using NumberToken = NumberConsumerForUnitlessValues<Length<R, V>, LengthValidator, LengthUnit::Px>;
  };
}
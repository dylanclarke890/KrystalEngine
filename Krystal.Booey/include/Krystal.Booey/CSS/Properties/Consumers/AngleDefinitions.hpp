#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSPrimitiveNumericTypesCanonicalization.hpp"

namespace krys::boo::css
{
  struct AngleValidator
  {
    KRYS_NODISCARD constexpr static Maybe<AngleUnit> Validate(CSSUnitType unitType, PropertyParserState &,
                                                              PropertyParserOptions) noexcept
    {
      return UnitTraits<AngleUnit>::Validate(unitType);
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(AngleRaw<R, V> raw, PropertyParserOptions) noexcept
    {
      return IsValidDimensionValue(raw,
                                   [&]
                                   {
                                     auto canonicalValue = Canonicalize(raw);
                                     return canonicalValue >= raw.Range.Min
                                            && canonicalValue <= raw.Range.Max;
                                   });
    }

    KRYS_NODISCARD static bool ShouldAcceptUnitlessValue(double value, PropertyParserState &state,
                                                         PropertyParserOptions options) noexcept
    {
      if (!value && options.UnitlessZeroAngle)
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
  struct ConsumerDefinition<Angle<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<Angle<R, V>>;
    using DimensionToken = DimensionConsumer<Angle<R, V>, AngleValidator>;
    using NumberToken = NumberConsumerForUnitlessValues<Angle<R, V>, AngleValidator, AngleUnit::Deg>;
  };
}
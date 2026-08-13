#pragma once

#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace Krys::HTML
{
  struct NumberValidator
  {
    KRYS_NODISCARD constexpr static Maybe<NumberUnit> Validate(CSSUnitType unitType, CSSPropertyParserState &,
                                                               CSSPropertyParserOptions) noexcept
    {
      return UnitTraits<NumberUnit>::Validate(unitType);
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(NumberRaw<R, V> raw, CSSPropertyParserOptions) noexcept
    {
      return IsValidCanonicalValue(raw);
    }
  };

  template <CSSRange R, typename V>
  struct ConsumerDefinition<Number<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<Number<R, V>>;
    using NumberToken = NumberConsumer<Number<R, V>, NumberValidator>;
  };
}
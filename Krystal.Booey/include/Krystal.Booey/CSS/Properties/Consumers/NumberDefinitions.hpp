#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace krys::boo::css
{
  struct NumberValidator
  {
    KRYS_NODISCARD constexpr static Maybe<NumberUnit> Validate(UnitType unitType, PropertyParserState &,
                                                               PropertyParserOptions) noexcept
    {
      return UnitTraits<NumberUnit>::Validate(unitType);
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(NumberRaw<R, V> raw, PropertyParserOptions) noexcept
    {
      return IsValidCanonicalValue(raw);
    }
  };

  template <Range R, typename V>
  struct ConsumerDefinition<Number<R, V>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<Number<R, V>>;
    using NumberToken = NumberConsumer<Number<R, V>, NumberValidator>;
  };
}
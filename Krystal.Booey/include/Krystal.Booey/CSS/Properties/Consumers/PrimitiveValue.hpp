#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaResolver.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveNumeric.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  // MARK: Resolver for users that want to get a PrimitiveValue result.

  /// Non-template base type for code sharing.
  struct PrimitiveValueResolverBase
  {
    KRYS_NODISCARD static RefPtr<PrimitiveValue> Resolve(NumericRaw auto value,
                                                            PropertyParserOptions = {}) noexcept
    {
      return PrimitiveValue::Create(value.Value, ToUnitType(value.Unit));
    }

    template <Range R, typename T>
    KRYS_NODISCARD static RefPtr<PrimitiveValue> Resolve(IntegerRaw<R, T> value,
                                                            PropertyParserOptions) noexcept
    {
      return PrimitiveValue::Create(value.Value);
    }

    KRYS_NODISCARD static RefPtr<PrimitiveValue> Resolve(Calc auto value,
                                                            PropertyParserOptions = {}) noexcept
    {
      (void)value;
      krys_not_implemented();
    }

    KRYS_NODISCARD static RefPtr<PrimitiveValue> Resolve(Numeric auto value,
                                                            PropertyParserOptions options = {}) noexcept
    {
      return SwitchOn(krys::move(value), [&](auto &&value) { return Resolve(krys::move(value), options); });
    }

    template <Range nR, Range pR, typename T>
    KRYS_NODISCARD static RefPtr<PrimitiveValue>
      Resolve(const NumberOrPercentageResolvedToNumber<nR, pR, T> &value,
              PropertyParserOptions options = {}) noexcept
    {
      return SwitchOn(
        value,
        [&](const Number<nR, T> &value) -> RefPtr<PrimitiveValue> { return Resolve(value, options); },
        [&](const Percentage<pR, T> &value) -> RefPtr<PrimitiveValue>
        {
          return SwitchOn(
            value, [&](const Percentage<pR, T>::Raw &raw) -> RefPtr<PrimitiveValue>
            { return PrimitiveValue::Create(raw.Value / 100.0, UnitType::Number); },
            [&](const Percentage<pR, T>::Calc &calc) -> RefPtr<PrimitiveValue>
            { return Resolve(calc, options); });
        });
    }
  };

  template <typename... Ts>
  struct PrimitiveValueResolver
      : MetaResolver<RefPtr<PrimitiveValue>, PrimitiveValueResolverBase, Ts...>
  {
    using MetaResolver<RefPtr<PrimitiveValue>, PrimitiveValueResolverBase, Ts...>::Resolve;
    using MetaResolver<RefPtr<PrimitiveValue>, PrimitiveValueResolverBase, Ts...>::ConsumeAndResolve;
  };
}
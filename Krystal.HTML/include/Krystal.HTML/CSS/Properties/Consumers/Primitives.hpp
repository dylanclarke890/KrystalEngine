#pragma once

#include "Krystal.HTML/CSS/Calc/CSSCalcValue.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaResolver.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumeric.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSUnevaluatedCalc.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  // MARK: Resolver for users that want to get a CSSPrimitiveValue result.

  /// Non-template base type for code sharing.
  struct CSSPrimitiveValueResolverBase
  {
    KRYS_NODISCARD static RefPtr<CSSPrimitiveValue> Resolve(NumericRaw auto value,
                                                            CSSPropertyParserOptions = {}) noexcept
    {
      return CreateRef<CSSPrimitiveValue>(value.Value, ToCSSUnitType(value.Unit));
    }

    template <CSSRange R, typename T>
    KRYS_NODISCARD static RefPtr<CSSPrimitiveValue> Resolve(IntegerRaw<R, T> value,
                                                            CSSPropertyParserOptions) noexcept
    {
      return CreateRef<CSSPrimitiveValue>(value.Value);
    }

    KRYS_NODISCARD static RefPtr<CSSPrimitiveValue> Resolve(Calc auto value,
                                                            CSSPropertyParserOptions = {}) noexcept
    {
      // TODO: Implement calc() expression evaluation
      return nullptr;
    }

    KRYS_NODISCARD static RefPtr<CSSPrimitiveValue> Resolve(Numeric auto value,
                                                            CSSPropertyParserOptions options = {}) noexcept
    {
      return SwitchOn(Krys::Move(value), [&](auto &&value) { return Resolve(Krys::Move(value), options); });
    }

    template <CSSRange nR, CSSRange pR, typename T>
    KRYS_NODISCARD static RefPtr<CSSPrimitiveValue>
      Resolve(const NumberOrPercentageResolvedToNumber<nR, pR, T> &value,
              CSSPropertyParserOptions options = {}) noexcept
    {
      return SwitchOn(
        value,
        [&](const Number<nR, T> &value) -> RefPtr<CSSPrimitiveValue> { return Resolve(value, options); },
        [&](const Percentage<pR, T> &value) -> RefPtr<CSSPrimitiveValue>
        {
          return SwitchOn(
            value, [&](const Percentage<pR, T>::Raw &raw) -> RefPtr<CSSPrimitiveValue>
            { return CreateRef<CSSPrimitiveValue>(raw.Value / 100.0, CSSUnitType::Number); },
            [&](const Percentage<pR, T>::Calc &calc) -> RefPtr<CSSPrimitiveValue>
            { return Resolve(calc, options); });
        });
    }
  };

  template <typename... Ts>
  struct CSSPrimitiveValueResolver
      : MetaResolver<RefPtr<CSSPrimitiveValue>, CSSPrimitiveValueResolverBase, Ts...>
  {
    using MetaResolver<RefPtr<CSSPrimitiveValue>, CSSPrimitiveValueResolverBase, Ts...>::Resolve;
    using MetaResolver<RefPtr<CSSPrimitiveValue>, CSSPrimitiveValueResolverBase, Ts...>::ConsumeAndResolve;
  };
}
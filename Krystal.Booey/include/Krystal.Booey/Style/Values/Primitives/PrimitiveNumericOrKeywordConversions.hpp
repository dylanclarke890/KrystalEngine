#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumericOrKeyword.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::style
{
  template <auto R, typename V, css::PrimitiveKeyword... Ks>
  struct ToCSS<PrimitiveNumericOrKeyword<LengthPercentage<R, V>, Ks...>>
  {
    using Result = css::PrimitiveNumericOrKeyword<css::LengthPercentage<R, V>, Ks...>;

    KRYS_NODISCARD Result operator()(const PrimitiveNumericOrKeyword<LengthPercentage<R, V>, Ks...> &value,
                                     const RenderStyle &style) noexcept
    {
      return krys::SwitchOn(
        value,
        [&](const typename LengthPercentage<R, V>::Dimension &length) -> Result
        {
          if constexpr (R.ZoomOptions == css::RangeZoomOptions::Default)
          {
            return css::LengthPercentageRaw<R, V> {length.Unit,
                                                   AdjustForZoom(length.UnresolvedValue(), style)};
          }
          else if constexpr (R.ZoomOptions == css::RangeZoomOptions::Unzoomed)
          {
            if (EvaluationTimeZoomEnabled(style))
            {
              return css::LengthPercentageRaw<R, V> {length.Unit, length.UnresolvedValue()};
            }

            return css::LengthPercentageRaw<R, V> {length.Unit,
                                                   AdjustForZoom(length.UnresolvedValue(), style)};
          }
        },
        [&](const typename LengthPercentage<R, V>::Percentage &percentage) -> Result
        { return css::LengthPercentageRaw<R, V> {percentage.Unit, percentage.Value}; },
        [&](const typename LengthPercentage<R, V>::Calc &calculation) -> Result
        {
          return css::UnevaluatedCalc<css::LengthPercentageRaw<R, V>> {
            css::calc::Value::Create(*calculation.ProtectedCalculation(), style)};
        },
        [&]<css::ValueId Id>(const css::KeywordValueConstant<Id> &identifier) -> Result
        { return ConvertToCSS(identifier, style); });
    }
  };

  template <css::Numeric N, css::PrimitiveKeyword... Ks>
  struct ToStyle<css::PrimitiveNumericOrKeyword<N, Ks...>>
  {
    using From = css::PrimitiveNumericOrKeyword<N, Ks...>;
    using To = typename ToStyleMapping<From>::type;

    template <typename... Rest>
    KRYS_NODISCARD To operator()(const From &value, Rest &&...rest) noexcept
    {
      return krys::SwitchOn(value, [&](const auto &alternative) -> To
                            { return ConvertToStyle(alternative, std::forward<Rest>(rest)...); });
    }

    // Implement `BuilderState` overload to explicitly forward to the `ToLengthConversionData` overload.
    template <typename... Rest>
    KRYS_NODISCARD To operator()(const From &value, const BuilderState &state, Rest &&...rest) noexcept
    {
      return ConvertToStyle(value, ConversionData<typename N::Raw>(state), std::forward<Rest>(rest)...);
    }
  };
}

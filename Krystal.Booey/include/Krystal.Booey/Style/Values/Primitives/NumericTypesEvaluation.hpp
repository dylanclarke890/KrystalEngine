#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Style/Calc/Tree.hpp"
#include "Krystal.Booey/Style/Calc/Value.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCalc.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Attributes.hpp"
#include <algorithm>

namespace krys::boo::style
{
  using namespace css::Literals;

#pragma region Length

  template <auto R, typename V, typename Result>
  struct Evaluation<Length<R, V>, Result>
  {
    KRYS_NODISCARD constexpr Result operator()(const Length<R, V> &value, ZoomNeeded token) noexcept
    requires(R.ZoomOptions == css::RangeZoomOptions::Default)
    {
      return Result(value.ResolveZoom(token));
    }

    KRYS_NODISCARD constexpr Result operator()(const Length<R, V> &value, ZoomFactor zoom) noexcept
    requires(R.ZoomOptions == css::RangeZoomOptions::Unzoomed)
    {
      return Result(value.ResolveZoom(zoom));
    }

    KRYS_NODISCARD constexpr Result operator()(const Length<R, V> &value, Result, ZoomNeeded token) noexcept
    requires(R.ZoomOptions == css::RangeZoomOptions::Default)
    {
      return Result(value.ResolveZoom(token));
    }

    KRYS_NODISCARD constexpr Result operator()(const Length<R, V> &value, Result, ZoomFactor zoom) noexcept
    requires(R.ZoomOptions == css::RangeZoomOptions::Unzoomed)
    {
      return Result(value.ResolveZoom(zoom));
    }
  };

#pragma endregion

#pragma region Percentage

  template <auto R, typename V, typename Result>
  struct Evaluation<Percentage<R, V>, Result>
  {
    KRYS_NODISCARD constexpr Result operator()(const Percentage<R, V> &percentage) noexcept
    {
      return Result(percentage.Value / 100.0);
    }

    KRYS_NODISCARD constexpr Result operator()(const Percentage<R, V> &percentage,
                                               Result referenceLength) noexcept
    {
      return Result(percentage.Value / 100.0 * referenceLength);
    }
  };

#pragma endregion

#pragma region Numeric

  template <NonCompositeNumeric StyleType, typename Result>
  struct Evaluation<StyleType, Result>
  {
    KRYS_NODISCARD constexpr Result operator()(const StyleType &value) noexcept
    {
      return Result(value.Value);
    }

    KRYS_NODISCARD constexpr Result operator()(const StyleType &value, Result) noexcept
    {
      return Result(value.Value);
    }
  };

#pragma endregion

#pragma region Calculation

  template <typename Result>
  struct Evaluation<Ref<calc::Value>, Result>
  {
    KRYS_NODISCARD Result operator()(Ref<calc::Value> calculation, Result referenceLength,
                                     ZoomFactor usedZoom) noexcept
    {
      return Result(calculation->Evaluate(referenceLength, usedZoom));
    }

    KRYS_NODISCARD Result operator()(Ref<calc::Value> calculation, Result referenceLength,
                                     ZoomNeeded token) noexcept
    {
      return Result(calculation->Evaluate(referenceLength, token));
    }
  };

  template <Calc Calculation, typename Result>
  struct Evaluation<Calculation, Result>
  {
    template <typename... Rest>
    KRYS_NODISCARD Result operator()(const Calculation &calculation, Result referenceLength, ZoomNeeded token,
                                     Rest &&...rest) noexcept
    requires(Calculation::Range.ZoomOptions == css::RangeZoomOptions::Default)
    {
      return Evaluate<Result>(calculation.ProtectedCalculation(), referenceLength, token,
                              std::forward<Rest>(rest)...);
    }

    template <typename... Rest>
    KRYS_NODISCARD Result operator()(const Calculation &calculation, Result referenceLength,
                                     ZoomFactor usedZoom, Rest &&...rest) noexcept
    requires(Calculation::Range.ZoomOptions == css::RangeZoomOptions::Unzoomed)
    {
      return Evaluate<Result>(calculation.ProtectedCalculation(), referenceLength, usedZoom,
                              std::forward<Rest>(rest)...);
    }
  };

#pragma endregion

#pragma region LengthPercentage

  template <auto R, typename V, typename Result>
  struct Evaluation<LengthPercentage<R, V>, Result>
  {
    KRYS_NODISCARD constexpr Result operator()(const LengthPercentage<R, V> &lengthPercentage,
                                               Result referenceLength, ZoomNeeded token) noexcept
    requires(R.ZoomOptions == css::RangeZoomOptions::Default)
    {
      return krys::SwitchOn(
        lengthPercentage, [&](const typename LengthPercentage<R, V>::Dimension &length) -> Result
        { return Evaluate<Result>(length, token); },
        [&](const typename LengthPercentage<R, V>::Percentage &percentage) -> Result
        { return Evaluate<Result>(percentage, referenceLength); },
        [&](const typename LengthPercentage<R, V>::Calc &calculation) -> Result
        { return Evaluate<Result>(calculation, referenceLength, token); });
    }

    KRYS_NODISCARD constexpr Result operator()(const LengthPercentage<R, V> &lengthPercentage,
                                               Result referenceLength, ZoomFactor zoom) noexcept
    requires(R.ZoomOptions == css::RangeZoomOptions::Unzoomed)
    {
      return krys::SwitchOn(
        lengthPercentage, [&](const typename LengthPercentage<R, V>::Dimension &length) -> Result
        { return Evaluate<Result>(length, zoom); },
        [&](const typename LengthPercentage<R, V>::Percentage &percentage) -> Result
        { return Evaluate<Result>(percentage, referenceLength); },
        [&](const typename LengthPercentage<R, V>::Calc &calculation) -> Result
        { return Evaluate<Result>(calculation, referenceLength, zoom); });
    }
  };

#pragma endregion

#pragma region NumberOrPercentageResolvedToNumber

  template <css::Range nR, css::Range pR, typename V, typename Result>
  struct Evaluation<NumberOrPercentageResolvedToNumber<nR, pR, V>, Result>
  {
    KRYS_NODISCARD constexpr Result
      operator()(const NumberOrPercentageResolvedToNumber<nR, pR, V> &value) noexcept
    {
      return Evaluate<Result>(value.Value);
    }

    KRYS_NODISCARD constexpr Result operator()(const NumberOrPercentageResolvedToNumber<nR, pR, V> &value,
                                               Result) noexcept
    {
      return Evaluate<Result>(value.Value);
    }
  };

#pragma endregion

#pragma region SpaceSeparatedPoint

  template <typename T>
  struct Evaluation<css::SpaceSeparatedPoint<T>, FloatPoint>
  {
    KRYS_NODISCARD FloatPoint operator()(const css::SpaceSeparatedPoint<T> &value,
                                         FloatSize referenceBox) noexcept
    requires HasTwoParameterEvaluate<T, float, float>
    {
      return {Evaluate<float>(value.x(), referenceBox.Width()),
              Evaluate<float>(value.y(), referenceBox.Height())};
    }

    KRYS_NODISCARD FloatPoint operator()(const css::SpaceSeparatedPoint<T> &value, FloatSize referenceBox,
                                         ZoomNeeded token) noexcept
    requires HasThreeParameterEvaluate<T, float, float, ZoomNeeded>
    {
      return {Evaluate<float>(value.x(), referenceBox.Width(), token),
              Evaluate<float>(value.y(), referenceBox.Height(), token)};
    }

    KRYS_NODISCARD FloatPoint operator()(const css::SpaceSeparatedPoint<T> &value, FloatSize referenceBox,
                                         ZoomFactor zoom) noexcept
    requires HasThreeParameterEvaluate<T, float, float, ZoomFactor>
    {
      return {Evaluate<float>(value.x(), referenceBox.Width(), zoom),
              Evaluate<float>(value.y(), referenceBox.Height(), zoom)};
    }
  };

  template <typename T>
  struct Evaluation<css::SpaceSeparatedPoint<T>, LayoutPoint>
  {
    KRYS_NODISCARD LayoutPoint operator()(const css::SpaceSeparatedPoint<T> &value,
                                          LayoutSize referenceBox) noexcept
    requires HasTwoParameterEvaluate<T, LayoutUnit, LayoutUnit>
    {
      return {Evaluate<LayoutUnit>(value.x(), referenceBox.Width()),
              Evaluate<LayoutUnit>(value.y(), referenceBox.Height())};
    }

    KRYS_NODISCARD LayoutPoint operator()(const css::SpaceSeparatedPoint<T> &value, LayoutSize referenceBox,
                                          ZoomNeeded token) noexcept
    requires HasThreeParameterEvaluate<T, LayoutUnit, LayoutUnit, ZoomNeeded>
    {
      return {Evaluate<LayoutUnit>(value.x(), referenceBox.Width(), token),
              Evaluate<LayoutUnit>(value.y(), referenceBox.Height(), token)};
    }

    KRYS_NODISCARD LayoutPoint operator()(const css::SpaceSeparatedPoint<T> &value, LayoutSize referenceBox,
                                          ZoomFactor zoom) noexcept
    requires HasThreeParameterEvaluate<T, LayoutUnit, LayoutUnit, ZoomFactor>
    {
      return {Evaluate<LayoutUnit>(value.x(), referenceBox.Width(), zoom),
              Evaluate<LayoutUnit>(value.y(), referenceBox.Height(), zoom)};
    }
  };

#pragma endregion

#pragma region SpaceSeparatedSize

  template <typename T>
  struct Evaluation<css::SpaceSeparatedSize<T>, FloatSize>
  {
    KRYS_NODISCARD FloatSize operator()(const css::SpaceSeparatedSize<T> &value,
                                        FloatSize referenceBox) noexcept
    requires HasTwoParameterEvaluate<T, float, float>
    {
      return {Evaluate<float>(value.Width(), referenceBox.Width()),
              Evaluate<float>(value.Height(), referenceBox.Height())};
    }

    KRYS_NODISCARD FloatSize operator()(const css::SpaceSeparatedSize<T> &value, FloatSize referenceBox,
                                        ZoomNeeded token) noexcept
    requires HasThreeParameterEvaluate<T, float, float, ZoomNeeded>
    {
      return {Evaluate<float>(value.Width(), referenceBox.Width(), token),
              Evaluate<float>(value.Height(), referenceBox.Height(), token)};
    }

    KRYS_NODISCARD FloatSize operator()(const css::SpaceSeparatedSize<T> &value, FloatSize referenceBox,
                                        ZoomFactor zoom) noexcept
    requires HasThreeParameterEvaluate<T, float, float, ZoomFactor>
    {
      return {Evaluate<float>(value.Width(), referenceBox.Width(), zoom),
              Evaluate<float>(value.Height(), referenceBox.Height(), zoom)};
    }
  };

  template <typename T>
  struct Evaluation<css::SpaceSeparatedSize<T>, LayoutSize>
  {
    KRYS_NODISCARD LayoutSize operator()(const css::SpaceSeparatedSize<T> &value,
                                         LayoutSize referenceBox) noexcept
    requires HasTwoParameterEvaluate<T, LayoutUnit, LayoutUnit>
    {
      return {Evaluate<LayoutUnit>(value.Width(), referenceBox.Width()),
              Evaluate<LayoutUnit>(value.Height(), referenceBox.Height())};
    }

    KRYS_NODISCARD LayoutSize operator()(const css::SpaceSeparatedSize<T> &value, LayoutSize referenceBox,
                                         ZoomNeeded token) noexcept
    requires HasThreeParameterEvaluate<T, LayoutUnit, LayoutUnit, ZoomNeeded>
    {
      return {Evaluate<LayoutUnit>(value.Width(), referenceBox.Width(), token),
              Evaluate<LayoutUnit>(value.Height(), referenceBox.Height(), token)};
    }

    KRYS_NODISCARD LayoutSize operator()(const css::SpaceSeparatedSize<T> &value, LayoutSize referenceBox,
                                         ZoomFactor zoom) noexcept
    requires HasThreeParameterEvaluate<T, LayoutUnit, LayoutUnit, ZoomFactor>
    {
      return {Evaluate<LayoutUnit>(value.Width(), referenceBox.Width(), zoom),
              Evaluate<LayoutUnit>(value.Height(), referenceBox.Height(), zoom)};
    }
  };

#pragma endregion

#pragma region MinimallySerialisingSpaceSeparatedSize

  template <typename T>
  struct Evaluation<css::MinimallySerialisingSpaceSeparatedSize<T>, FloatSize>
  {
    KRYS_NODISCARD FloatSize operator()(const css::MinimallySerialisingSpaceSeparatedSize<T> &value,
                                        FloatSize referenceBox) noexcept
    requires HasTwoParameterEvaluate<T, float, float>
    {
      return {Evaluate<float>(value.Width(), referenceBox.Width()),
              Evaluate<float>(value.Height(), referenceBox.Height())};
    }

    KRYS_NODISCARD FloatSize operator()(const css::MinimallySerialisingSpaceSeparatedSize<T> &value,
                                        FloatSize referenceBox, ZoomNeeded token) noexcept
    requires HasThreeParameterEvaluate<T, float, float, ZoomNeeded>
    {
      return {Evaluate<float>(value.Width(), referenceBox.Width(), token),
              Evaluate<float>(value.Height(), referenceBox.Height(), token)};
    }

    KRYS_NODISCARD FloatSize operator()(const css::MinimallySerialisingSpaceSeparatedSize<T> &value,
                                        FloatSize referenceBox, ZoomFactor zoom) noexcept
    requires HasThreeParameterEvaluate<T, float, float, ZoomFactor>
    {
      return {Evaluate<float>(value.Width(), referenceBox.Width(), zoom),
              Evaluate<float>(value.Height(), referenceBox.Height(), zoom)};
    }
  };

  template <typename T>
  struct Evaluation<css::MinimallySerialisingSpaceSeparatedSize<T>, LayoutSize>
  {
    KRYS_NODISCARD LayoutSize operator()(const css::MinimallySerialisingSpaceSeparatedSize<T> &value,
                                         LayoutSize referenceBox) noexcept
    requires HasTwoParameterEvaluate<T, LayoutUnit, LayoutUnit>
    {
      return {Evaluate<LayoutUnit>(value.Width(), referenceBox.Width()),
              Evaluate<LayoutUnit>(value.Height(), referenceBox.Height())};
    }

    KRYS_NODISCARD LayoutSize operator()(const css::MinimallySerialisingSpaceSeparatedSize<T> &value,
                                         LayoutSize referenceBox, ZoomNeeded token) noexcept
    requires HasThreeParameterEvaluate<T, LayoutUnit, LayoutUnit, ZoomNeeded>
    {
      return {Evaluate<LayoutUnit>(value.Width(), referenceBox.Width(), token),
              Evaluate<LayoutUnit>(value.Height(), referenceBox.Height(), token)};
    }

    KRYS_NODISCARD LayoutSize operator()(const css::MinimallySerialisingSpaceSeparatedSize<T> &value,
                                         LayoutSize referenceBox, ZoomFactor zoom) noexcept
    requires HasThreeParameterEvaluate<T, LayoutUnit, LayoutUnit, ZoomFactor>
    {
      return {Evaluate<LayoutUnit>(value.Width(), referenceBox.Width(), zoom),
              Evaluate<LayoutUnit>(value.Height(), referenceBox.Height(), zoom)};
    }
  };

#pragma endregion

#pragma region Calculated Evaluations

  /// @brief Convert to `calc(100% - value)`.
  template <auto R, typename V>
  KRYS_NODISCARD LengthPercentage<R, V> Reflect(const LengthPercentage<R, V> &value) noexcept
  {
    using Result = LengthPercentage<R, V>;
    using Dimension = typename Result::Dimension;
    using Percentage = typename Result::Percentage;
    using Calc = typename Result::Calc;

    return krys::SwitchOn(
      value,
      [&](const Dimension &dimension) -> Result
      {
        // If `value` is 0, we can avoid the `calc` altogether.
        if (dimension == 0_css_px)
        {
          return 100_css_percentage;
        }

        // Turn this into a calc expression: `calc(100% - value)`.
        return Calc {calc::Subtract(calc::CreatePercentage(100), CopyCalculation(dimension))};
      },
      [&](const Percentage &percentage) -> Result
      {
        // If `value` is a percentage, we can avoid the `calc` altogether.
        return 100_css_percentage - percentage.Value;
      },
      [&](const Calc &calculation) -> Result
      {
        // Turn this into a calc expression: `calc(100% - value)`.
        return Calc {calc::Subtract(calc::CreatePercentage(100), CopyCalculation(calculation))};
      });
  }

  /// @brief Merges the two ranges, `aR` and `bR`, creating a union of their ranges.
  KRYS_NODISCARD consteval css::Range MergeRanges(css::Range aR, css::Range bR) noexcept
  {
    return css::Range {std::min(aR.Min, bR.Min), std::max(aR.Max, bR.Max)};
  }

  /// @brief Convert to `calc(100% - (a + b))`.
  ///
  /// Returns a LengthPercentage with range, `resultR`, equal to union of the two input ranges `aR` and `bR`.
  template <auto aR, auto bR, typename V>
  KRYS_NODISCARD LengthPercentage<MergeRanges(aR, bR), V>
    ReflectSum(const LengthPercentage<aR, V> &a, const LengthPercentage<bR, V> &b) noexcept
  {
    constexpr auto resultR = MergeRanges(aR, bR);

    using Result = LengthPercentage<resultR, V>;
    using CalcResult = typename Result::Calc;
    using PercentageA = typename LengthPercentage<aR, V>::Percentage;
    using PercentageB = typename LengthPercentage<bR, V>::Percentage;

    bool aIsZero = a.IsKnownZero();
    bool bIsZero = b.IsKnownZero();

    // If both `a` and `b` are 0, turn this into a calc expression: `calc(100% - (0 + 0))` aka `100%`.
    if (aIsZero && bIsZero)
    {
      return 100_css_percentage;
    }

    // If just `a` is 0, we can just consider the case of `calc(100% - b)`.
    if (aIsZero)
    {
      return krys::SwitchOn(
        b,
        [&](const PercentageB &percentage) -> Result
        {
          // And if `b` is a percent, we can avoid the `calc` altogether.
          return 100_css_percentage - percentage.Value;
        },
        [&](const auto &alternative) -> Result
        {
          // Otherwise, turn this into a calc expression: `calc(100% - b)`.
          return CalcResult {calc::Subtract(calc::CreatePercentage(100), CopyCalculation(alternative))};
        });
    }

    // If just `b` is 0, we can just consider the case of `calc(100% - a)`.
    if (bIsZero)
    {
      return krys::SwitchOn(
        a,
        [&](const PercentageA &percentage) -> Result
        {
          // And if `a` is a percent, we can avoid the `calc` altogether.
          return 100_css_percentage - percentage.Value;
        },
        [&](const auto &alternative) -> Result
        {
          // Otherwise, turn this into a calc expression: `calc(100% - a)`.
          return CalcResult {calc::Subtract(calc::CreatePercentage(100), CopyCalculation(alternative))};
        });
    }

    // If both and `a` and `b` are percentages, we can avoid the `calc` altogether.
    if (krys::HoldsAlternative<PercentageA>(a) && krys::HoldsAlternative<PercentageB>(b))
    {
      return 100_css_percentage - (get<PercentageA>(a).Value + get<PercentageB>(b).Value);
    }

    // Otherwise, turn this into a calc expression: `calc(100% - (a + b))`.
    return CalcResult {
      calc::Subtract(calc::CreatePercentage(100), calc::Add(CopyCalculation(a), CopyCalculation(b)))};
  }

#pragma endregion
}

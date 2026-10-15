#pragma once

#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesCanonicalization.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Symbol.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Calc/Value.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::style
{
  // Out of line to avoid inclusion of the `RenderStyle` getters.
  KRYS_NODISCARD float AdjustForZoom(float, const RenderStyle &) noexcept;
  KRYS_NODISCARD bool EvaluationTimeZoomEnabled(const RenderStyle &) noexcept;
  KRYS_NODISCARD bool EvaluationTimeZoomEnabled(const BuilderState &) noexcept;

#pragma region Conversion Data specialization

  template <typename T>
  struct ConversionDataSpecializer
  {
    KRYS_NODISCARD css::ToLengthConversionData operator()(const BuilderState &state) const noexcept
    {
      return state.CssToLengthConversionData();
    }
  };

  template <auto R, typename V>
  struct ConversionDataSpecializer<css::LengthRaw<R, V>>
  {
    KRYS_NODISCARD css::ToLengthConversionData operator()(const BuilderState &state) const noexcept
    {
      if constexpr (R.ZoomOptions == css::RangeZoomOptions::Default)
      {
        return state.UseSVGZoomRulesForLength() ? state.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f)
                                                : state.CssToLengthConversionData();
      }
      else if constexpr (R.ZoomOptions == css::RangeZoomOptions::Unzoomed)
      {
        if (EvaluationTimeZoomEnabled(state))
        {
          return state.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f, R.ZoomOptions);
        }

        return state.UseSVGZoomRulesForLength() ? state.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f)
                                                : state.CssToLengthConversionData();
      }
    }
  };

  template <typename T>
  KRYS_NODISCARD css::ToLengthConversionData ConversionData(const BuilderState &state) noexcept
  {
    return ConversionDataSpecializer<T> {}(state);
  }

#pragma endregion

#pragma region Type maps

  // MARK: Raw -> CSS

  template <typename>
  struct RawToCSSMapping;

  template <auto R, typename V>
  struct RawToCSSMapping<css::IntegerRaw<R, V>>
  {
    using type = css::Integer<R, V>;
  };

  template <auto R, typename V>
  struct RawToCSSMapping<css::NumberRaw<R, V>>
  {
    using type = css::Number<R, V>;
  };

  template <auto R, typename V>
  struct RawToCSSMapping<css::PercentageRaw<R, V>>
  {
    using type = css::Percentage<R, V>;
  };

  template <auto R, typename V>
  struct RawToCSSMapping<css::AngleRaw<R, V>>
  {
    using type = css::Angle<R, V>;
  };

  template <auto R, typename V>
  struct RawToCSSMapping<css::LengthRaw<R, V>>
  {
    using type = css::Length<R, V>;
  };

  template <auto R, typename V>
  struct RawToCSSMapping<css::TimeRaw<R, V>>
  {
    using type = css::Time<R, V>;
  };

  template <auto R, typename V>
  struct RawToCSSMapping<css::FrequencyRaw<R, V>>
  {
    using type = css::Frequency<R, V>;
  };

  template <auto R, typename V>
  struct RawToCSSMapping<css::ResolutionRaw<R, V>>
  {
    using type = css::Resolution<R, V>;
  };

  template <auto R, typename V>
  struct RawToCSSMapping<css::FlexRaw<R, V>>
  {
    using type = css::Flex<R, V>;
  };

  template <auto R, typename V>
  struct RawToCSSMapping<css::AnglePercentageRaw<R, V>>
  {
    using type = css::AnglePercentage<R, V>;
  };

  template <auto R, typename V>
  struct RawToCSSMapping<css::LengthPercentageRaw<R, V>>
  {
    using type = css::LengthPercentage<R, V>;
  };

  // MARK: CSS -> Raw

  template <css::Numeric T>
  struct CSSToRawMapping
  {
    using type = typename T::Raw;
  };

#pragma endregion

#pragma region Raw canonicalization

  // MARK: Length

  KRYS_NODISCARD double CanonicalizeLength(double, css::LengthUnit,
                                           css::NoConversionDataRequiredTag) noexcept;
  KRYS_NODISCARD double CanonicalizeLength(double, css::LengthUnit,
                                           const css::ToLengthConversionData &) noexcept;
  KRYS_NODISCARD float ClampLengthToAllowedLimits(double) noexcept;
  KRYS_NODISCARD float CanonicalizeAndClampLength(double, css::LengthUnit,
                                                  css::NoConversionDataRequiredTag) noexcept;
  KRYS_NODISCARD float CanonicalizeAndClampLength(double, css::LengthUnit,
                                                  const css::ToLengthConversionData &) noexcept;

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD constexpr Integer<R, V> Canonicalize(const css::IntegerRaw<R, V> &raw,
                                                      css::NoConversionDataRequiredTag, Rest &&...) noexcept
  {
    return {css::RoundForImpreciseConversion<V>(raw.Value)};
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD constexpr Integer<R, V> Canonicalize(const css::IntegerRaw<R, V> &raw,
                                                      const css::ToLengthConversionData &,
                                                      Rest &&...rest) noexcept
  {
    return Canonicalize(raw, css::NoConversionDataRequiredTag {}, std::forward<Rest>(rest)...);
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD constexpr Number<R, V> Canonicalize(const css::NumberRaw<R, V> &raw,
                                                     css::NoConversionDataRequiredTag, Rest &&...) noexcept
  {
    return {raw.Value};
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD constexpr Number<R, V> Canonicalize(const css::NumberRaw<R, V> &raw,
                                                     const css::ToLengthConversionData &,
                                                     Rest &&...rest) noexcept
  {
    return Canonicalize(raw, css::NoConversionDataRequiredTag {}, std::forward<Rest>(rest)...);
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD constexpr Percentage<R, V>
    Canonicalize(const css::PercentageRaw<R, V> &raw, css::NoConversionDataRequiredTag, Rest &&...) noexcept
  {
    return {static_cast<V>(raw.Value)};
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD constexpr Percentage<R, V> Canonicalize(const css::PercentageRaw<R, V> &raw,
                                                         const css::ToLengthConversionData &,
                                                         Rest &&...rest) noexcept
  {
    return Canonicalize(raw, css::NoConversionDataRequiredTag {}, std::forward<Rest>(rest)...);
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD Angle<R, V> Canonicalize(const css::AngleRaw<R, V> &raw, css::NoConversionDataRequiredTag,
                                          Rest &&...) noexcept
  {
    return {static_cast<V>(css::Canonicalize(raw))};
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD Angle<R, V> Canonicalize(const css::AngleRaw<R, V> &raw, const css::ToLengthConversionData &,
                                          Rest &&...rest) noexcept
  {
    return Canonicalize(raw, css::NoConversionDataRequiredTag {}, std::forward<Rest>(rest)...);
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD Length<R, V> Canonicalize(const css::LengthRaw<R, V> &raw,
                                           css::NoConversionDataRequiredTag token, Rest &&...) noexcept
  {
    krys_debug_assert(!css::ConversionToCanonicalUnitRequiresConversionData(raw.Unit));

    return {CanonicalizeAndClampLength(raw.Value, raw.Unit, token)};
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD Length<R, V> Canonicalize(const css::LengthRaw<R, V> &raw,
                                           const css::ToLengthConversionData &conversionData,
                                           Rest &&...) noexcept
  {
#if KRYS_DEBUG
    css::ComputedStyleDependencies dependencies;
    css::CollectComputedStyleDependencies(dependencies, raw);
    krys_debug_assert(dependencies.CanResolveDependenciesWithConversionData(conversionData));
#endif

    return {CanonicalizeAndClampLength(raw.Value, raw.Unit, conversionData)};
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD Time<R, V> Canonicalize(const css::TimeRaw<R, V> &raw, css::NoConversionDataRequiredTag,
                                         Rest &&...) noexcept
  {
    return {static_cast<V>(css::Canonicalize(raw))};
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD Time<R, V> Canonicalize(const css::TimeRaw<R, V> &raw, const css::ToLengthConversionData &,
                                         Rest &&...rest) noexcept
  {
    return Canonicalize(raw, css::NoConversionDataRequiredTag {}, std::forward<Rest>(rest)...);
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD Frequency<R, V> Canonicalize(const css::FrequencyRaw<R, V> &raw,
                                              css::NoConversionDataRequiredTag, Rest &&...) noexcept
  {
    return {static_cast<V>(css::Canonicalize(raw))};
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD Frequency<R, V> Canonicalize(const css::FrequencyRaw<R, V> &raw,
                                              const css::ToLengthConversionData &, Rest &&...rest) noexcept
  {
    return Canonicalize(raw, css::NoConversionDataRequiredTag {}, std::forward<Rest>(rest)...);
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD Resolution<R, V> Canonicalize(const css::ResolutionRaw<R, V> &raw,
                                               css::NoConversionDataRequiredTag, Rest &&...) noexcept
  {
    return {static_cast<V>(css::Canonicalize(raw))};
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD Resolution<R, V> Canonicalize(const css::ResolutionRaw<R, V> &raw,
                                               const css::ToLengthConversionData &, Rest &&...rest) noexcept
  {
    return Canonicalize(raw, css::NoConversionDataRequiredTag {}, std::forward<Rest>(rest)...);
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD constexpr Flex<R, V> Canonicalize(const css::FlexRaw<R, V> &raw,
                                                   css::NoConversionDataRequiredTag, Rest &&...) noexcept
  {
    return {static_cast<V>(raw.Value)};
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD constexpr Flex<R, V>
    Canonicalize(const css::FlexRaw<R, V> &raw, const css::ToLengthConversionData &, Rest &&...rest) noexcept
  {
    return Canonicalize(raw, css::NoConversionDataRequiredTag {}, std::forward<Rest>(rest)...);
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD AnglePercentage<R, V> Canonicalize(const css::AnglePercentageRaw<R, V> &raw,
                                                    css::NoConversionDataRequiredTag token,
                                                    Rest &&...rest) noexcept
  {
    return css::SwitchOnUnitType(
      raw.Unit, [&](css::PercentageUnit) -> AnglePercentage<R, V>
      { return {Canonicalize(css::PercentageRaw<R, V> {raw.Value}, token, std::forward<Rest>(rest)...)}; },
      [&](css::AngleUnit angleUnit) -> AnglePercentage<R, V>
      {
        return {Canonicalize(css::AngleRaw<R, V> {angleUnit, raw.Value}, token, std::forward<Rest>(rest)...)};
      });
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD AnglePercentage<R, V> Canonicalize(const css::AnglePercentageRaw<R, V> &raw,
                                                    const css::ToLengthConversionData &,
                                                    Rest &&...rest) noexcept
  {
    return Canonicalize(raw, css::NoConversionDataRequiredTag {}, std::forward<Rest>(rest)...);
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD LengthPercentage<R, V> Canonicalize(const css::LengthPercentageRaw<R, V> &raw,
                                                     css::NoConversionDataRequiredTag token,
                                                     Rest &&...rest) noexcept
  {
    return css::SwitchOnUnitType(
      raw.Unit, [&](css::PercentageUnit) -> LengthPercentage<R, V>
      { return Canonicalize(css::PercentageRaw<R, V> {raw.Value}, token, std::forward<Rest>(rest)...); },
      [&](css::LengthUnit lengthUnit) -> LengthPercentage<R, V>
      {
        return Canonicalize(css::LengthRaw<R, V> {lengthUnit, raw.Value}, token, std::forward<Rest>(rest)...);
      });
  }

  template <auto R, typename V, typename... Rest>
  KRYS_NODISCARD LengthPercentage<R, V> Canonicalize(const css::LengthPercentageRaw<R, V> &raw,
                                                     const css::ToLengthConversionData &conversionData,
                                                     Rest &&...rest) noexcept
  {
    return css::SwitchOnUnitType(
      raw.Unit,
      [&](css::PercentageUnit) -> LengthPercentage<R, V>
      {
        return Canonicalize(css::PercentageRaw<R, V> {raw.Value}, conversionData,
                            std::forward<Rest>(rest)...);
      },
      [&](css::LengthUnit lengthUnit) -> LengthPercentage<R, V>
      {
        return Canonicalize(css::LengthRaw<R, V> {lengthUnit, raw.Value}, conversionData,
                            std::forward<Rest>(rest)...);
      });
  }

#pragma endregion

#pragma region Conversion from Style to CSS

  // Length requires a specialized implementation due to zoom adjustment.
  template <auto R, typename V>
  struct ToCSS<Length<R, V>>
  {
    KRYS_NODISCARD css::Length<R, V> operator()(const Length<R, V> &value, const RenderStyle &style) noexcept
    {
      if constexpr (R.ZoomOptions == css::RangeZoomOptions::Default)
      {
        return css::LengthRaw<R, V> {value.Unit, AdjustForZoom(value.UnresolvedValue(), style)};
      }
      else if constexpr (R.ZoomOptions == css::RangeZoomOptions::Unzoomed)
      {
        if (EvaluationTimeZoomEnabled(style))
        {
          return css::LengthRaw<R, V> {value.Unit, value.UnresolvedValue()};
        }

        return css::LengthRaw<R, V> {value.Unit, AdjustForZoom(value.UnresolvedValue(), style)};
      }
    }
  };

  template <auto R, typename V>
  struct ToCSS<UnevaluatedCalc<css::AnglePercentage<R, V>>>
  {
    KRYS_NODISCARD typename css::AnglePercentage<R, V>::Calc
      operator()(const UnevaluatedCalc<css::AnglePercentage<R, V>> &value, const RenderStyle &style) noexcept
    {
      return typename css::AnglePercentage<R, V>::Calc {
        css::calc::Value::Create(*value.ProtectedCalculation(), style)};
    }
  };

  template <auto R, typename V>
  struct ToCSS<UnevaluatedCalc<css::LengthPercentage<R, V>>>
  {
    KRYS_NODISCARD typename css::LengthPercentage<R, V>::Calc
      operator()(const UnevaluatedCalc<css::LengthPercentage<R, V>> &value, const RenderStyle &style) noexcept
    {
      return typename css::LengthPercentage<R, V>::Calc {
        css::calc::Value::Create(*value.ProtectedCalculation(), style)};
    }
  };

  // AnglePercentage / LengthPercentage require specialized implementations due to additional `calc` field.
  template <auto R, typename V>
  struct ToCSS<AnglePercentage<R, V>>
  {
    KRYS_NODISCARD css::AnglePercentage<R, V> operator()(const AnglePercentage<R, V> &value,
                                                         const RenderStyle &style) noexcept
    {
      return krys::SwitchOn(
        value, [&](const Angle<R, V> &angle) -> css::AnglePercentage<R, V>
        { return typename css::AnglePercentage<R, V>::Raw {angle.Unit, angle.Value}; },
        [&](const Percentage<R, V> &percentage) -> css::AnglePercentage<R, V>
        { return typename css::AnglePercentage<R, V>::Raw {percentage.Unit, percentage.Value}; },
        [&](const typename AnglePercentage<R, V>::Calc &calculation) -> css::AnglePercentage<R, V>
        {
          return typename css::AnglePercentage<R, V>::Calc {
            css::calc::Value::Create(*calculation.ProtectedCalculation(), style)};
        });
    }
  };

  template <auto R, typename V>
  struct ToCSS<LengthPercentage<R, V>>
  {
    KRYS_NODISCARD css::LengthPercentage<R, V> operator()(const LengthPercentage<R, V> &value,
                                                          const RenderStyle &style) noexcept
    {
      return krys::SwitchOn(
        value,
        [&](const typename LengthPercentage<R, V>::Dimension &length) -> css::LengthPercentage<R, V>
        {
          if constexpr (R.ZoomOptions == css::RangeZoomOptions::Default)
          {
            return typename css::LengthPercentage<R, V>::Raw {length.Unit,
                                                              AdjustForZoom(length.UnresolvedValue(), style)};
          }
          else if constexpr (R.ZoomOptions == css::RangeZoomOptions::Unzoomed)
          {
            return typename css::LengthPercentage<R, V>::Raw {length.Unit, length.UnresolvedValue()};
          }
        },
        [&](const typename LengthPercentage<R, V>::Percentage &percentage) -> css::LengthPercentage<R, V>
        { return typename css::LengthPercentage<R, V>::Raw {percentage.Unit, percentage.Value}; },
        [&](const typename LengthPercentage<R, V>::Calc &calculation) -> css::LengthPercentage<R, V>
        {
          return typename css::LengthPercentage<R, V>::Calc {
            css::calc::Value::Create(*calculation.ProtectedCalculation(), style)};
        });
    }
  };

  // Partial specialization for remaining numeric types.
  template <Numeric StyleType>
  struct ToCSS<StyleType>
  {
    KRYS_NODISCARD typename StyleType::CSS operator()(const StyleType &value, const RenderStyle &) noexcept
    {
      return {value.Unit, value.Value};
    }
  };

  // NumberOrPercentageResolvedToNumber requires specialization due to asymmetric representations.
  template <auto nR, auto pR, typename V>
  struct ToCSS<NumberOrPercentageResolvedToNumber<nR, pR, V>>
  {
    KRYS_NODISCARD css::NumberOrPercentageResolvedToNumber<nR, pR, V>
      operator()(const NumberOrPercentageResolvedToNumber<nR, pR, V> &value,
                 const RenderStyle &style) noexcept
    {
      return {ConvertToCSS(value.Value, style)};
    }
  };

#pragma endregion

#pragma region Conversion from CSS to Style

  namespace detail_conversions
  {
    // AnglePercentage and LengthPercentage share their calc canonicalization; `TDimensionRaw` is the
    // dimension half (`css::AngleRaw` or `css::LengthRaw`).
    template <typename To, typename TDimensionRaw, typename TPercentageRaw, typename From, typename... Rest>
    KRYS_NODISCARD To DimensionPercentageCalcToStyle(css::Category dimensionCategory,
                                                     css::Category dimensionPercentageCategory,
                                                     const From &value, Rest &&...rest) noexcept
    {
      // NOTE: Simplification is needed here for the case of the user using the Typed CSSOM
      // to explicitly specify a CSSMath* value for a specified value.

      Ref<css::calc::Value> simplifiedCalc = value.ProtectedCalc()->CopySimplified(rest...);

      // FIXME: This ASSERT and the following extra cases for the dimension and Category::Percentage
      // should go away once the typed CSSOM learns to set the correct category when creating internal
      // representations of CSSMath* types.

      krys_debug_assert(simplifiedCalc->GetCategory() == dimensionPercentageCategory
                        || simplifiedCalc->GetCategory() == dimensionCategory
                        || simplifiedCalc->GetCategory() == css::Category::Percentage);

      if (simplifiedCalc->GetCategory() == dimensionCategory)
      {
        return Canonicalize(TDimensionRaw {To::Dimension::Unit, simplifiedCalc->DoubleValue(rest...)},
                            std::forward<Rest>(rest)...);
      }

      if (simplifiedCalc->GetCategory() == css::Category::Percentage)
      {
        if (krys::HoldsAlternative<css::calc::Percentage>(simplifiedCalc->GetTree().Root))
        {
          return Canonicalize(TPercentageRaw {simplifiedCalc->DoubleValue(rest...)},
                              std::forward<Rest>(rest)...);
        }

        return typename To::Calc {simplifiedCalc->CreateCalculationValue(rest...)};
      }

      if (!simplifiedCalc->GetTree().Type.PercentHint)
      {
        return Canonicalize(TDimensionRaw {To::Dimension::Unit, simplifiedCalc->DoubleValue(rest...)},
                            std::forward<Rest>(rest)...);
      }

      if (krys::HoldsAlternative<css::calc::Percentage>(simplifiedCalc->GetTree().Root))
      {
        return Canonicalize(TPercentageRaw {simplifiedCalc->DoubleValue(rest...)},
                            std::forward<Rest>(rest)...);
      }

      return typename To::Calc {simplifiedCalc->CreateCalculationValue(rest...)};
    }
  }

  template <auto R, typename V>
  struct ToStyle<css::UnevaluatedCalc<css::AnglePercentageRaw<R, V>>>
  {
    using From = css::UnevaluatedCalc<css::AnglePercentageRaw<R, V>>;
    using To = AnglePercentage<R, V>;

    template <typename... Rest>
    KRYS_NODISCARD To operator()(const From &value, Rest &&...rest) noexcept
    {
      return detail_conversions::DimensionPercentageCalcToStyle<To, css::AngleRaw<R, V>,
                                                                css::PercentageRaw<R, V>>(
        css::Category::Angle, css::Category::AnglePercentage, value, std::forward<Rest>(rest)...);
    }
  };

  template <auto R, typename V>
  struct ToStyle<css::UnevaluatedCalc<css::LengthPercentageRaw<R, V>>>
  {
    using From = css::UnevaluatedCalc<css::LengthPercentageRaw<R, V>>;
    using To = LengthPercentage<R, V>;

    template <typename... Rest>
    KRYS_NODISCARD To operator()(const From &value, Rest &&...rest) noexcept
    {
      return detail_conversions::DimensionPercentageCalcToStyle<To, css::LengthRaw<R, V>,
                                                                css::PercentageRaw<R, V>>(
        css::Category::Length, css::Category::LengthPercentage, value, std::forward<Rest>(rest)...);
    }
  };

  // Partial specialization for remaining numeric types.

  template <css::NumericRaw RawType>
  struct ToStyle<RawType>
  {
    using From = RawType;
    using To = typename ToStyleMapping<typename RawToCSSMapping<RawType>::type>::type;

    template <typename... Rest>
    KRYS_NODISCARD To operator()(const From &value, Rest &&...rest) noexcept
    {
      return {Canonicalize(value, std::forward<Rest>(rest)...)};
    }
  };

  template <css::NumericRaw RawType>
  struct ToStyle<css::UnevaluatedCalc<RawType>>
  {
    using From = css::UnevaluatedCalc<RawType>;
    using To = typename ToStyleMapping<typename RawToCSSMapping<RawType>::type>::type;

    template <typename... Rest>
    KRYS_NODISCARD To operator()(const From &value, Rest &&...rest) noexcept
    {
      return {Canonicalize(RawType {To::Unit, value.Evaluate(From::Category, rest...)}, rest...)};
    }
  };

  template <css::Numeric NumericType>
  struct ToStyle<NumericType>
  {
    using From = NumericType;
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
      return ConvertToStyle(value, ConversionData<typename From::Raw>(state), std::forward<Rest>(rest)...);
    }
  };

  // NumberOrPercentageResolvedToNumber, as the name implies, resolves its percentage to a number.
  template <auto nR, auto pR, typename V>
  struct ToStyle<css::NumberOrPercentageResolvedToNumber<nR, pR, V>>
  {
    using From = css::NumberOrPercentageResolvedToNumber<nR, pR, V>;
    using To = NumberOrPercentageResolvedToNumber<nR, pR, V>;

    template <typename... Rest>
    KRYS_NODISCARD To operator()(const From &value, Rest &&...rest) noexcept
    {
      return krys::SwitchOn(value, [&](const auto &numberOrPercentage) -> To
                            { return {ConvertToStyle(numberOrPercentage, std::forward<Rest>(rest)...)}; });
    }
  };

#pragma endregion
}

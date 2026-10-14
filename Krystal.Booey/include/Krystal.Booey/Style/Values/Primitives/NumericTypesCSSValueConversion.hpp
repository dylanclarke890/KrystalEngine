#pragma once

#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Units.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  template <css::Range R, typename V>
  struct CSSValueConversion<Integer<R, V>>
  {
    KRYS_NODISCARD Integer<R, V> operator()(BuilderState &builderState,
                                            const css::PrimitiveValue &value) noexcept
    {
      return {css::ClampToRange<R, V>(value.ResolveAsNumber<V>(builderState.CssToLengthConversionData()))};
    }

    KRYS_NODISCARD Integer<R, V> operator()(BuilderState &builderState, const css::Value &value) noexcept
    {
      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(builderState, value);
      if (!primitiveValue)
      {
        return 0_css_integer;
      }
      return (*this)(builderState, *primitiveValue);
    }
  };

  template <css::Range R, typename V>
  struct CSSValueConversion<Number<R, V>>
  {
    KRYS_NODISCARD Number<R, V> operator()(BuilderState &builderState,
                                           const css::PrimitiveValue &value) noexcept
    {
      return {css::ClampToRange<R, V>(value.ResolveAsNumber<V>(builderState.CssToLengthConversionData()))};
    }

    KRYS_NODISCARD Number<R, V> operator()(BuilderState &builderState, const css::Value &value) noexcept
    {
      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(builderState, value);
      if (!primitiveValue)
      {
        return 0_css_number;
      }
      return (*this)(builderState, *primitiveValue);
    }
  };

  template <css::Range R, typename V>
  struct CSSValueConversion<Percentage<R, V>>
  {
    KRYS_NODISCARD Percentage<R, V> operator()(BuilderState &builderState,
                                               const css::PrimitiveValue &value) noexcept
    {
      return {
        css::ClampToRange<R, V>(value.ResolveAsPercentage<V>(builderState.CssToLengthConversionData()))};
    }

    KRYS_NODISCARD Percentage<R, V> operator()(BuilderState &builderState, const css::Value &value) noexcept
    {
      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(builderState, value);
      if (!primitiveValue)
      {
        return 0_css_percentage;
      }
      return (*this)(builderState, *primitiveValue);
    }
  };

  template <css::Range R, typename V>
  struct CSSValueConversion<Angle<R, V>>
  {
    KRYS_NODISCARD Angle<R, V> operator()(BuilderState &builderState,
                                          const css::PrimitiveValue &value) noexcept
    {
      return {css::ClampToRange<R, V>(value.ResolveAsAngle<V>(builderState.CssToLengthConversionData()))};
    }

    KRYS_NODISCARD Angle<R, V> operator()(BuilderState &builderState, const css::Value &value) noexcept
    {
      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(builderState, value);
      if (!primitiveValue)
      {
        return 0_css_deg;
      }
      return (*this)(builderState, *primitiveValue);
    }
  };

  template <css::Range R, typename V>
  struct CSSValueConversion<Length<R, V>>
  {
    KRYS_NODISCARD static css::ToLengthConversionData
      SelectConversionData(BuilderState &builderState) noexcept
    {
      if constexpr (R.ZoomOptions == css::RangeZoomOptions::Default)
      {
        return builderState.UseSVGZoomRulesForLength()
                 ? builderState.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f)
                 : builderState.CssToLengthConversionData();
      }
      else if constexpr (R.ZoomOptions == css::RangeZoomOptions::Unzoomed)
      {
        if (EvaluationTimeZoomEnabled(builderState))
        {
          return builderState.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f, R.ZoomOptions);
        }

        return builderState.UseSVGZoomRulesForLength()
                 ? builderState.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f, R.ZoomOptions)
                 : builderState.CssToLengthConversionData();
      }
    }

    KRYS_NODISCARD Length<R, V> operator()(BuilderState &builderState,
                                           const css::PrimitiveValue &value) noexcept
    {
      auto conversionData = SelectConversionData(builderState);
      return {css::ClampToRange<R, V>(value.ResolveAsLength<V>(conversionData))};
    }

    KRYS_NODISCARD Length<R, V> operator()(BuilderState &builderState, const css::Value &value) noexcept
    {
      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(builderState, value);
      if (!primitiveValue)
      {
        return 0_css_px;
      }
      return (*this)(builderState, *primitiveValue);
    }

    KRYS_NODISCARD Length<R, V> operator()(const css::ToLengthConversionData &conversionData,
                                           const css::PrimitiveValue &value) noexcept
    {
      return {css::ClampToRange<R, V>(value.ResolveAsLength<V>(conversionData))};
    }
  };

  template <css::Range R, typename V>
  struct CSSValueConversion<Time<R, V>>
  {
    KRYS_NODISCARD Time<R, V> operator()(BuilderState &builderState,
                                         const css::PrimitiveValue &value) noexcept
    {
      return {css::ClampToRange<R, V>(value.ResolveAsTime<V>(builderState.CssToLengthConversionData()))};
    }

    KRYS_NODISCARD Time<R, V> operator()(BuilderState &builderState, const css::Value &value) noexcept
    {
      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(builderState, value);
      if (!primitiveValue)
      {
        return 0_css_s;
      }
      return (*this)(builderState, *primitiveValue);
    }
  };

  template <css::Range R, typename V>
  struct CSSValueConversion<Resolution<R, V>>
  {
    KRYS_NODISCARD Resolution<R, V> operator()(BuilderState &builderState,
                                               const css::PrimitiveValue &value) noexcept
    {
      return {
        css::ClampToRange<R, V>(value.ResolveAsResolution<V>(builderState.CssToLengthConversionData()))};
    }

    KRYS_NODISCARD Resolution<R, V> operator()(BuilderState &builderState, const css::Value &value) noexcept
    {
      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(builderState, value);
      if (!primitiveValue)
      {
        return 0_css_dppx;
      }
      return (*this)(builderState, *primitiveValue);
    }
  };

  template <css::Range R, typename V>
  struct CSSValueConversion<Flex<R, V>>
  {
    KRYS_NODISCARD Flex<R, V> operator()(BuilderState &builderState,
                                         const css::PrimitiveValue &value) noexcept
    {
      return {css::ClampToRange<R, V>(value.ResolveAsFlex<V>(builderState.CssToLengthConversionData()))};
    }

    KRYS_NODISCARD Flex<R, V> operator()(BuilderState &builderState, const css::Value &value) noexcept
    {
      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(builderState, value);
      if (!primitiveValue)
      {
        return 0_css_fr;
      }
      return (*this)(builderState, *primitiveValue);
    }
  };

  template <css::Range R, typename V>
  struct CSSValueConversion<LengthPercentage<R, V>>
  {
    using StyleType = LengthPercentage<R, V>;

    KRYS_NODISCARD static css::ToLengthConversionData
      SelectConversionData(BuilderState &builderState) noexcept
    {
      if constexpr (StyleType::Dimension::Range.ZoomOptions == css::RangeZoomOptions::Default)
      {
        return builderState.UseSVGZoomRulesForLength()
                 ? builderState.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f)
                 : builderState.CssToLengthConversionData();
      }
      else if constexpr (StyleType::Dimension::Range.ZoomOptions == css::RangeZoomOptions::Unzoomed)
      {
        if (EvaluationTimeZoomEnabled(builderState))
        {
          return builderState.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f);
        }

        return builderState.UseSVGZoomRulesForLength()
                 ? builderState.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f)
                 : builderState.CssToLengthConversionData();
      }
    }

    KRYS_NODISCARD StyleType operator()(BuilderState &builderState, const css::PrimitiveValue &value) noexcept
    {
      auto conversionData = SelectConversionData(builderState);
      if (value.IsPercentage())
      {
        return typename StyleType::Percentage {
          css::ClampToRange<R, V>(value.ResolveAsPercentage<V>(conversionData))};
      }
      if (value.IsCalculatedPercentageWithLength())
      {
        return typename StyleType::Calc {
          value.ProtectedCSSCalcValue()->CreateCalculationValue(conversionData, css::SymbolTable {})};
      }
      return
        typename StyleType::Dimension {css::ClampToRange<R, V>(value.ResolveAsLength<V>(conversionData))};
    }

    KRYS_NODISCARD StyleType operator()(BuilderState &builderState, const css::Value &value) noexcept
    {
      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(builderState, value);
      if (!primitiveValue)
      {
        return 0_css_px;
      }
      return (*this)(builderState, *primitiveValue);
    }
  };

  template <css::Range nR, css::Range pR, typename V>
  struct CSSValueConversion<NumberOrPercentage<nR, pR, V>>
  {
    using StyleType = NumberOrPercentage<nR, pR, V>;

    KRYS_NODISCARD StyleType operator()(BuilderState &builderState, const css::PrimitiveValue &value) noexcept
    {
      const auto &conversionData = builderState.CssToLengthConversionData();
      if (value.IsPercentage())
      {
        return typename StyleType::Percentage {
          css::ClampToRange<pR, V>(value.ResolveAsPercentage<V>(conversionData))};
      }
      return typename StyleType::Number {css::ClampToRange<nR, V>(value.ResolveAsNumber<V>(conversionData))};
    }

    KRYS_NODISCARD StyleType operator()(BuilderState &builderState, const css::Value &value) noexcept
    {
      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(builderState, value);
      if (!primitiveValue)
      {
        return 0_css_number;
      }
      return (*this)(builderState, *primitiveValue);
    }
  };

  template <css::Range nR, css::Range pR, typename V>
  struct CSSValueConversion<NumberOrPercentageResolvedToNumber<nR, pR, V>>
  {
    using StyleType = NumberOrPercentageResolvedToNumber<nR, pR, V>;

    KRYS_NODISCARD StyleType operator()(BuilderState &builderState, const css::PrimitiveValue &value) noexcept
    {
      const auto &conversionData = builderState.CssToLengthConversionData();
      if (value.IsPercentage())
      {
        return typename StyleType::Percentage {
          css::ClampToRange<pR, V>(value.ResolveAsPercentage<V>(conversionData))};
      }
      return typename StyleType::Number {css::ClampToRange<nR, V>(value.ResolveAsNumber<V>(conversionData))};
    }

    KRYS_NODISCARD StyleType operator()(BuilderState &builderState, const css::Value &value) noexcept
    {
      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(builderState, value);
      if (!primitiveValue)
      {
        return 0_css_number;
      }
      return (*this)(builderState, *primitiveValue);
    }
  };
}

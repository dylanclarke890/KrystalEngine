#include "Krystal.Booey/Style/Values/Box/Margin.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"

namespace krys::boo::style
{
  MarginEdge CSSValueConversion<MarginEdge>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Auto {};
    }

    return operator()(state, *primitiveValue);
  }

  MarginEdge CSSValueConversion<MarginEdge>::operator()(BuilderState &state,
                                                        const css::PrimitiveValue &primitiveValue) noexcept
  {
    if (primitiveValue.ValueId() == css::ValueId::Auto)
    {
      return css::keywords::Auto {};
    }

    auto cssToLengthConversionDataWithTextZoomFactorIfNeeded =
      [](BuilderState &state, bool isFontIndependentUnit) -> css::ToLengthConversionData
    {
      auto usedZoom = 1.0f;

      if (isFontIndependentUnit)
      {
        usedZoom = EvaluationTimeZoomEnabled(state) ? 1.0f : state.Style().UsedZoom();
      }
      else
      {
        usedZoom = state.ZoomWithTextZoomFactor();
      }

      return state.CssToLengthConversionData().CopyWithAdjustedZoom(usedZoom,
                                                                    MarginEdge::Fixed::Range.ZoomOptions);
    };

    auto conversionData = state.UseSVGZoomRulesForLength()
                            ? state.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f)
                            : cssToLengthConversionDataWithTextZoomFactorIfNeeded(
                                state, primitiveValue.IsFontIndependentLength());

    if (primitiveValue.IsLength())
    {
      return MarginEdge {typename MarginEdge::Fixed {
                           css::ClampToRange<MarginEdge::Fixed::Range, float>(
                             primitiveValue.ResolveAsLength(conversionData), css::MinValueForCssLength,
                             css::MaxValueForCssLength),
                         },
                         primitiveValue.PrimitiveType() == css::UnitType::QuirkyEm};
    }

    if (primitiveValue.IsPercentage())
    {
      return MarginEdge {typename MarginEdge::Percentage {
        css::ClampToRange<MarginEdge::Percentage::Range, float>(
          primitiveValue.ResolveAsPercentage(conversionData)),
      }};
    }

    if (primitiveValue.IsCalculatedPercentageWithLength())
    {
      return MarginEdge {typename MarginEdge::Calc {
        primitiveValue.ProtectedCSSCalcValue()->CreateCalculationValue(conversionData, css::SymbolTable {})}};
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return css::keywords::Auto {};
  }
}

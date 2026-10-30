#include "Krystal.Booey/Style/Values/Inline/LineHeight.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"

#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  LineHeight CSSValueConversion<LineHeight>::operator()(BuilderState &state, const css::Value &value,
                                                        float multiplier) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Normal {};
    }

    return operator()(state, *primitiveValue, multiplier);
  }

  LineHeight CSSValueConversion<LineHeight>::operator()(BuilderState &state,
                                                        const css::PrimitiveValue &primitiveValue,
                                                        float multiplier) noexcept
  {
    auto valueID = primitiveValue.ValueId();
    if (valueID == css::ValueId::Normal || css::PropertyParserHelpers::IsSystemFontShorthand(valueID))
    {
      return css::keywords::Normal {};
    }

    auto conversionData = state.CssToLengthConversionData().CopyForLineHeight(state.ZoomWithTextZoomFactor());

    // If EvaluationTimeZoom is not enabled then we will scale the lengths in the
    // calc values when we create the CalculationValue below by using the zoom from conversionData.
    // To avoid double zooming when we evaluate the calc expression we need to make sure
    // we have a ZoomFactor of 1.0. Otherwise, we defer to whatever is on the conversionData
    // since EvaluationTimeZoom will set the appropriate value.
    auto zoomFactor = [&]
    {
      if (!state.Style().EvaluationTimeZoomEnabled())
      {
        return style::ZoomFactor {1.0f, state.Style().DeviceScaleFactor()};
      }
      return style::ZoomFactor {conversionData.Zoom(), state.Style().DeviceScaleFactor()};
    };

    if (primitiveValue.IsLength() || primitiveValue.IsCalculatedPercentageWithLength())
    {
      double fixedValue = 0;
      if (primitiveValue.IsLength())
      {
        fixedValue = primitiveValue.ResolveAsLength(conversionData);
      }
      else
      {
        fixedValue = primitiveValue.ProtectedCSSCalcValue()
                       ->CreateCalculationValue(conversionData, css::SymbolTable {})
                       ->Evaluate(state.Style().GetFontDescription().ComputedSizeForRangeZoomOption(
                                    conversionData.RangeZoomOption()),
                                  zoomFactor());
      }

      if (multiplier != 1.0f)
      {
        fixedValue *= multiplier;
      }

      return LineHeight::Fixed {css::ClampToRange<LineHeight::Fixed::Range, float>(
        fixedValue, css::MinValueForCssLength, css::MaxValueForCssLength)};
    }

    // Line-height percentages need to inherit as if they were Fixed pixel values. In the example:
    // <div style="font-size: 10px; line-height: 150%;"><div style="font-size: 100px;"></div></div>
    // the inner element should have line-height of 15px. However, in this example:
    // <div style="font-size: 10px; line-height: 1.5;"><div style="font-size: 100px;"></div></div>
    // the inner element should have a line-height of 150px. Therefore, we map percentages to Fixed
    // values and raw numbers to percentages.
    if (primitiveValue.IsPercentage())
    {
      // FIXME: percentage should not be restricted to an integer here.
      auto textZoom = EvaluationTimeZoomEnabled(state) ? conversionData.Zoom() : 1.0f;
      return LineHeight::Fixed {css::ClampToRange<LineHeight::Fixed::Range, float>(
        (state.Style().GetFontDescription().ComputedSizeForRangeZoomOption(conversionData.RangeZoomOption())
         * primitiveValue.ResolveAsPercentage<int>(conversionData) * textZoom)
          / 100.0,
        css::MinValueForCssLength, css::MaxValueForCssLength)};
    }

    if (primitiveValue.IsNumber())
    {
      return LineHeight::Percentage {css::ClampToRange<LineHeight::Percentage::Range, float>(
        primitiveValue.ResolveAsNumber(conversionData) * 100.0)};
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return css::keywords::Normal {};
  }

  // MARK: - Blending

  bool Blending<LineHeight>::CanBlend(const LineHeight &a, const LineHeight &b) noexcept
  {
    return a.HasSameType(b) || (a.IsCalculated() && b.IsSpecified()) || (b.IsCalculated() && a.IsSpecified());
  }

  bool Blending<LineHeight>::RequiresInterpolationForAccumulativeIteration(const LineHeight &a,
                                                                           const LineHeight &b) noexcept
  {
    return !a.HasSameType(b) || a.IsCalculated() || b.IsCalculated();
  }

  LineHeight Blending<LineHeight>::Blend(const LineHeight &a, const LineHeight &b,
                                         const BlendingContext &context) noexcept
  {
    if (!a.IsSpecified() || !b.IsSpecified())
    {
      return context.Progress < 0.5 ? a : b;
    }

    if (a.IsCalculated() || b.IsCalculated() || !a.HasSameType(b))
    {
      return LengthWrapperBlendingSupport<LineHeight>::BlendMixedSpecifiedTypes(a, b, context);
    }

    if (!context.Progress && context.IsReplace())
    {
      return a;
    }

    if (context.Progress == 1 && context.IsReplace())
    {
      return b;
    }

    auto resultType = b._value.Type();

    krys_debug_assert(resultType == LineHeight::IndexForPercentage
                      || resultType == LineHeight::IndexForFixed);

    if (resultType == LineHeight::IndexForPercentage)
    {
      return style::Blend(LineHeight::Percentage {a._value.Value()},
                          LineHeight::Percentage {b._value.Value()}, context);
    }
    else
    {
      return style::Blend(LineHeight::Fixed {a._value.Value()}, LineHeight::Fixed {b._value.Value()},
                          context);
    }
  }
}

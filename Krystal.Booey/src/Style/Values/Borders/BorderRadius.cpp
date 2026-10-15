#include "Krystal.Booey/Style/Values/Borders/BorderRadius.hpp"

#include "Krystal.Booey/CSS/Values/Primitives/Units.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  css::BorderRadius ToCSS<BorderRadius>::operator()(const BorderRadius &value,
                                                    const RenderStyle &style) noexcept
  {
    return {
      .Horizontal {
        ConvertToCSS(value.TopLeft().Width(), style),
        ConvertToCSS(value.TopRight().Width(), style),
        ConvertToCSS(value.BottomRight().Width(), style),
        ConvertToCSS(value.BottomLeft().Width(), style),
      },
      .Vertical {
        ConvertToCSS(value.TopLeft().Height(), style),
        ConvertToCSS(value.TopRight().Height(), style),
        ConvertToCSS(value.BottomRight().Height(), style),
        ConvertToCSS(value.BottomLeft().Height(), style),
      },
    };
  }

  BorderRadius ToStyle<css::BorderRadius>::operator()(const css::BorderRadius &value,
                                                      const BuilderState &state) noexcept
  {
    return {
      ConvertToStyle(value.TopLeft(), state),
      ConvertToStyle(value.TopRight(), state),
      ConvertToStyle(value.BottomLeft(), state),
      ConvertToStyle(value.BottomRight(), state),
    };
  }

  BorderRadiusValue CSSValueConversion<BorderRadiusValue>::operator()(BuilderState &state,
                                                                      const css::Value &value) noexcept
  {
    if (!value.IsValuePairValue())
    {
      return {0_css_px, 0_css_px};
    }

    auto pair = RequiredPairDowncast<css::PrimitiveValue>(state, value);
    if (!pair)
    {
      return {0_css_px, 0_css_px};
    }

    return {ConvertCSSValueToStyle<LengthPercentage<css::NonNegative>>(state, *pair->first),
            ConvertCSSValueToStyle<LengthPercentage<css::NonNegative>>(state, *pair->second)};
  }

  // MARK: - Evaluation

  FloatRoundedRect::Radii Evaluation<BorderRadius, FloatRoundedRect::Radii>::operator()(
    const BorderRadius &value, FloatSize referenceSize, ZoomNeeded token) noexcept
  {
    return {
      Evaluate<FloatSize>(value.TopLeft(), referenceSize, token),
      Evaluate<FloatSize>(value.TopRight(), referenceSize, token),
      Evaluate<FloatSize>(value.BottomLeft(), referenceSize, token),
      Evaluate<FloatSize>(value.BottomRight(), referenceSize, token),
    };
  }

  LayoutRoundedRect::Radii Evaluation<BorderRadius, LayoutRoundedRect::Radii>::operator()(
    const BorderRadius &value, LayoutSize referenceSize, ZoomNeeded token) noexcept
  {
    return {
      Evaluate<LayoutSize>(value.TopLeft(), referenceSize, token),
      Evaluate<LayoutSize>(value.TopRight(), referenceSize, token),
      Evaluate<LayoutSize>(value.BottomLeft(), referenceSize, token),
      Evaluate<LayoutSize>(value.BottomRight(), referenceSize, token),
    };
  }
}

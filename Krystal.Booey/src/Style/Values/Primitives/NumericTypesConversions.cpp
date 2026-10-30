#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthResolution.hpp"
#include "Krystal.Core/Maths/Base.hpp"

namespace krys::boo::style
{
  // MARK: Length Canonicalization

  double CanonicalizeLength(double value, css::LengthUnit unit, css::NoConversionDataRequiredTag) noexcept
  {
    return ComputeNonCalcLengthDouble(value, unit, {});
  }

  double CanonicalizeLength(double value, css::LengthUnit unit,
                            const css::ToLengthConversionData &conversionData) noexcept
  {
    return ComputeNonCalcLengthDouble(value, unit, conversionData);
  }

  float ClampLengthToAllowedLimits(double value) noexcept
  {
    return ClampTo<float>(static_cast<float>(value), css::MinValueForCssLength, css::MaxValueForCssLength);
  }

  float CanonicalizeAndClampLength(double value, css::LengthUnit unit,
                                   css::NoConversionDataRequiredTag token) noexcept
  {
    return ClampLengthToAllowedLimits(CanonicalizeLength(value, unit, token));
  }

  float CanonicalizeAndClampLength(double value, css::LengthUnit unit,
                                   const css::ToLengthConversionData &conversionData) noexcept
  {
    return ClampLengthToAllowedLimits(CanonicalizeLength(value, unit, conversionData));
  }

  // MARK: ToCSS utilities

  float AdjustForZoom(float value, const RenderStyle &style) noexcept
  {
    return AdjustFloatForAbsoluteZoom(value, style);
  }

  bool EvaluationTimeZoomEnabled(const RenderStyle &style) noexcept
  {
    return style.EvaluationTimeZoomEnabled();
  }

  bool EvaluationTimeZoomEnabled(const BuilderState &state) noexcept
  {
    return state.GetDocument().GetSettings().Values.EvaluationTimeZoomEnabled;
  }
}

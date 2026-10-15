#include "Krystal.Booey/Style/Values/SVG/SVGGlyphOrientationHorizontal.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include <cmath>

namespace krys::boo::style
{
  SVGGlyphOrientationHorizontal
    CSSValueConversion<SVGGlyphOrientationHorizontal>::operator()(BuilderState &state,
                                                                  const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return SVGGlyphOrientationHorizontal::Degrees0;
    }

    auto angle =
      std::abs(std::fmod(ConvertCSSValueToStyle<style::Angle<>>(state, *primitiveValue).Value, 360.0f));
    if (angle <= 45.0f || angle > 315.0f)
    {
      return SVGGlyphOrientationHorizontal::Degrees0;
    }
    if (angle > 45.0f && angle <= 135.0f)
    {
      return SVGGlyphOrientationHorizontal::Degrees90;
    }
    if (angle > 135.0f && angle <= 225.0f)
    {
      return SVGGlyphOrientationHorizontal::Degrees180;
    }
    return SVGGlyphOrientationHorizontal::Degrees270;
  }
}

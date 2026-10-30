#include "Krystal.Booey/Style/Values/SVG/SVGGlyphOrientationVertical.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include <cmath>

namespace krys::boo::style
{
  SVGGlyphOrientationVertical
    CSSValueConversion<SVGGlyphOrientationVertical>::operator()(BuilderState &state,
                                                                const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return SVGGlyphOrientationVertical::Auto;
    }

    if (primitiveValue->ValueId() == css::ValueId::Auto)
    {
      return SVGGlyphOrientationVertical::Auto;
    }

    auto angle =
      std::abs(std::fmod(ConvertCSSValueToStyle<style::Angle<>>(state, *primitiveValue).Value, 360.0f));
    if (angle <= 45.0f || angle > 315.0f)
    {
      return SVGGlyphOrientationVertical::Degrees0;
    }
    if (angle > 45.0f && angle <= 135.0f)
    {
      return SVGGlyphOrientationVertical::Degrees90;
    }
    if (angle > 135.0f && angle <= 225.0f)
    {
      return SVGGlyphOrientationVertical::Degrees180;
    }
    return SVGGlyphOrientationVertical::Degrees270;
  }
}

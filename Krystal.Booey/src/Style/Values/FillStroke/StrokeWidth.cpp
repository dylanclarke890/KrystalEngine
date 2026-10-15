#include "Krystal.Booey/Style/Values/FillStroke/StrokeWidth.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  StrokeWidth CSSValueConversion<StrokeWidth>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return 0_css_px;
    }

    if (primitiveValue->IsNumberOrInteger())
    {
      return StrokeWidthLength {StrokeWidthLength::Fixed {
        ConvertCSSValueToStyle<Number<css::NonNegative, float>>(state, *primitiveValue).Value}};
    }
    return ConvertCSSValueToStyle<StrokeWidthLength>(state, *primitiveValue);
  }
}

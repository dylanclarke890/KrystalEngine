#include "Krystal.Booey/Style/Values/Color/Opacity.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  Opacity CSSValueConversion<Opacity>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return 1.0f;
    }

    auto opacity = primitiveValue->ValueDividingBy100IfPercentage<float>(state.CssToLengthConversionData());
    return css::ClampToRange<Opacity::Number::Range, Opacity::Number::ResolvedValueType>(opacity);
  }
}

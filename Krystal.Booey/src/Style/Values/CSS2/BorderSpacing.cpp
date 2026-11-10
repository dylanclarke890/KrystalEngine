#include "Krystal.Booey/Style/Values/CSS2/BorderSpacing.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  BorderSpacing CSSValueConversion<BorderSpacing>::operator()(BuilderState &state,
                                                              const css::Value &value) noexcept
  {
    auto pair = RequiredPairDowncast<css::PrimitiveValue>(state, value);
    if (!pair)
    {
      return {0_css_px, 0_css_px};
    }

    return {ConvertCSSValueToStyle<BorderSpacingLength>(state, *pair->first),
            ConvertCSSValueToStyle<BorderSpacingLength>(state, *pair->second)};
  }
}

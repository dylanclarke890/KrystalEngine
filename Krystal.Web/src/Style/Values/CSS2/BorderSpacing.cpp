#include "Krystal.Web/Style/Values/CSS2/BorderSpacing.hpp"

#include "Krystal.Web/Css/Values/PrimitiveValue.hpp"
#include "Krystal.Web/Css/Values/ValuePair.hpp"
#include "Krystal.Web/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Web/Style/Builder/BuilderState.hpp"
#include "Krystal.Web/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::web::style
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

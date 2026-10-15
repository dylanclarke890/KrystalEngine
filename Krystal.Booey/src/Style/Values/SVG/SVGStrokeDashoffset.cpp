#include "Krystal.Booey/Style/Values/SVG/SVGStrokeDashoffset.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  SVGStrokeDashoffset CSSValueConversion<SVGStrokeDashoffset>::operator()(BuilderState &state,
                                                                          const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return 0_css_px;
    }

    if (primitiveValue->IsNumberOrInteger())
    {
      return SVGStrokeDashoffsetLength {SVGStrokeDashoffsetLength::Fixed {
        ConvertCSSValueToStyle<Number<css::All, float>>(state, *primitiveValue).Value}};
    }
    return ConvertCSSValueToStyle<SVGStrokeDashoffsetLength>(state, *primitiveValue);
  }
}

#include "Krystal.Booey/Style/Values/SVG/SVGStrokeDasharray.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  SVGStrokeDasharrayValue
    CSSValueConversion<SVGStrokeDasharrayValue>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return 0_css_px;
    }

    if (primitiveValue->IsNumberOrInteger())
    {
      return SVGStrokeDasharrayValueLength {SVGStrokeDasharrayValueLength::Fixed {
        ConvertCSSValueToStyle<Number<css::All, float>>(state, *primitiveValue).Value}};
    }
    return ConvertCSSValueToStyle<SVGStrokeDasharrayValueLength>(state, *primitiveValue);
  }

  // MARK: - Blending

  SVGStrokeDasharray Blending<SVGStrokeDasharray>::Blend(const SVGStrokeDasharray &a,
                                                         const SVGStrokeDasharray &b,
                                                         const BlendingContext &context) noexcept
  {
    auto aLength = a.size();
    auto bLength = b.size();
    if (!aLength || !bLength)
    {
      return context.Progress < 0.5 ? a : b;
    }

    auto resultLength = aLength;
    if (aLength != bLength)
    {
      if (!(std::max(aLength, bLength) % std::min(aLength, bLength)))
      {
        resultLength = std::max(aLength, bLength);
      }
      else
      {
        resultLength = aLength * bLength;
      }
    }

    return SVGStrokeDasharrayList::createWithSizeFromGenerator(
      resultLength, [&](auto i) { return style::Blend(a[i % aLength], b[i % bLength], context); });
  }
}

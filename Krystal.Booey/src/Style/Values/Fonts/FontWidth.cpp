#include "Krystal.Booey/Style/Values/Fonts/FontWidth.hpp"

#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  FontWidth CSSValueConversion<FontWidth>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    const auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Normal {};
    }

    switch (auto valueID = primitiveValue->ValueId(); valueID)
    {
      case css::ValueId::Invalid:
        return ConvertCSSValueToStyle<FontWidth::Percentage>(state, *primitiveValue);
      case css::ValueId::UltraCondensed: return css::keywords::UltraCondensed {};
      case css::ValueId::ExtraCondensed: return css::keywords::ExtraCondensed {};
      case css::ValueId::Condensed:      return css::keywords::Condensed {};
      case css::ValueId::SemiCondensed:  return css::keywords::SemiCondensed {};
      case css::ValueId::Normal:         return css::keywords::Normal {};
      case css::ValueId::SemiExpanded:   return css::keywords::SemiExpanded {};
      case css::ValueId::Expanded:       return css::keywords::Expanded {};
      case css::ValueId::ExtraExpanded:  return css::keywords::ExtraExpanded {};
      case css::ValueId::UltraExpanded:  return css::keywords::UltraExpanded {};
      default:
        if (css::PropertyParserHelpers::IsSystemFontShorthand(valueID))
        {
          return css::keywords::Normal {};
        }

        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return css::keywords::Normal {};
    }
  }

  // MARK: Blending

  FontWidth Blending<FontWidth>::Blend(const FontWidth &a, const FontWidth &b,
                                       const BlendingContext &context) noexcept
  {
    return style::Blend(a.GetPercentage(), b.GetPercentage(), context);
  }
}

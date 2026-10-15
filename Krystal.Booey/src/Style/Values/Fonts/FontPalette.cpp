#include "Krystal.Booey/Style/Values/Fonts/FontPalette.hpp"

#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  FontPalette CSSValueConversion<FontPalette>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    const auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Normal {};
    }

    switch (auto valueID = primitiveValue->ValueId(); valueID)
    {
      case css::ValueId::Normal: return css::keywords::Normal {};
      case css::ValueId::Light:  return css::keywords::Light {};
      case css::ValueId::Dark:   return css::keywords::Dark {};
      case css::ValueId::Invalid:
        return ConvertCSSValueToStyle<css::CustomIdentifier>(state, *primitiveValue);
      default:
        if (css::PropertyParserHelpers::IsSystemFontShorthand(valueID))
        {
          return css::keywords::Normal {};
        }

        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return css::keywords::Normal {};
    }
  }
}

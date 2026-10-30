#include "Krystal.Booey/Style/Values/Fonts/FontWeight.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"

#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascadeDescription.hpp"
#include "Krystal.Booey/Graphics/SystemFontDatabase.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  FontWeight CSSValueConversion<FontWeight>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    const auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Normal {};
    }

    switch (auto valueID = primitiveValue->ValueId(); valueID)
    {
      case css::ValueId::Invalid: return ConvertCSSValueToStyle<FontWeight::Number>(state, *primitiveValue);
      case css::ValueId::Normal:  return css::keywords::Normal {};
      case css::ValueId::Bold:    return css::keywords::Bold {};
      case css::ValueId::Bolder:
        return FontCascadeDescription::BolderWeight(state.ParentStyle().GetFontDescription().Weight());
      case css::ValueId::Lighter:
        return FontCascadeDescription::LighterWeight(state.ParentStyle().GetFontDescription().Weight());
      default:
        if (css::PropertyParserHelpers::IsSystemFontShorthand(valueID))
        {
          return SystemFontDatabase::Singleton().SystemFontShorthandWeight(
            css::PropertyParserHelpers::LowerFontShorthand(valueID));
        }

        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return css::keywords::Normal {};
    }
  }

  // MARK: Blending

  FontWeight Blending<FontWeight>::Blend(const FontWeight &a, const FontWeight &b,
                                         const BlendingContext &context) noexcept
  {
    return style::Blend(a.GetNumber(), b.GetNumber(), context);
  }
}

#include "Krystal.Booey/Style/ResolveForFont.hpp"
#include "Krystal.Booey/CSS/FontSelectionValueInlines.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Values/FontStyleWithAngleValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include <algorithm>

namespace krys::boo::style
{
  // MARK: - 'font-weight'

  FontSelectionValue FontWeightFromCSSValueDeprecated(const css::Value &value) noexcept
  {
    const auto &primitiveValue = Downcast<css::PrimitiveValue>(value);

    if (primitiveValue.IsNumber())
    {
      return FontSelectionValue(std::clamp(primitiveValue.ResolveAsNumberDeprecated<float>(), 1.0f, 1000.0f));
    }

    krys_debug_assert(primitiveValue.IsValueId());
    switch (primitiveValue.ValueId())
    {
      case css::ValueId::Normal:  return NormalWeightValue();
      case css::ValueId::Bold:
      case css::ValueId::Bolder:  return BoldWeightValue();
      case css::ValueId::Lighter: return LightWeightValue();
      default:                    krys_debug_assert(false); return NormalWeightValue();
    }
  }

  // MARK: - 'font-stretch'

  FontSelectionValue FontStretchFromCSSValueDeprecated(const css::Value &value) noexcept
  {
    const auto &primitiveValue = Downcast<css::PrimitiveValue>(value);

    if (primitiveValue.IsPercentage())
    {
      return FontSelectionValue::ClampFloat(primitiveValue.ResolveAsPercentageDeprecated<float>());
    }

    krys_debug_assert(primitiveValue.IsValueId());
    if (auto width = FontWidthValue(primitiveValue.ValueId()))
    {
      return *width;
    }

    krys_debug_assert(css::PropertyParserHelpers::IsSystemFontShorthand(primitiveValue.ValueId()));
    return NormalWidthValue();
  }

  // MARK: - 'font-style'

  FontSelectionValue FontStyleAngleFromCSSValueDeprecated(const css::Value &value) noexcept
  {
    return NormalizedFontItalicValue(Downcast<css::PrimitiveValue>(value).ResolveAsAngleDeprecated<float>());
  }

  Maybe<FontSelectionValue>
    FontStyleAngleFromCSSFontStyleWithAngleValueDeprecated(const css::FontStyleWithAngleValue &value) noexcept
  {
    if (css::RequiresConversionData(value.GetObliqueAngle()))
    {
      return null;
    }
    return FontSelectionValue {
      static_cast<float>(ConvertToStyleNoConversionDataRequired(value.GetObliqueAngle()).Value)};
  }

  Maybe<FontSelectionValue> FontStyleFromCSSValueDeprecated(const css::Value &value) noexcept
  {
    if (const auto *fontStyleValue = DynamicDowncast<css::FontStyleWithAngleValue>(value))
    {
      return FontStyleAngleFromCSSFontStyleWithAngleValueDeprecated(*fontStyleValue);
    }

    auto valueID = css::GetValueId(value);
    if (valueID == css::ValueId::Normal)
    {
      return null;
    }

    krys_debug_assert(valueID == css::ValueId::Italic || valueID == css::ValueId::Oblique);
    return ItalicValue();
  }
}

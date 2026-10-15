#include "Krystal.Booey/Style/Values/Fonts/FontStyle.hpp"

#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Values/FontStyleWithAngleValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  FontStyle CSSValueConversion<FontStyle>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (const auto *fontStyleValue = DynamicDowncast<css::FontStyleWithAngleValue>(value))
    {
      return ConvertToStyle(fontStyleValue->GetObliqueAngle(), state);
    }

    const auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Normal {};
    }

    switch (auto valueID = primitiveValue->ValueId(); valueID)
    {
      case css::ValueId::Normal:  return css::keywords::Normal {};
      case css::ValueId::Italic:  return css::keywords::Italic {};
      case css::ValueId::Oblique: return css::keywords::Oblique {};
      default:
        if (css::PropertyParserHelpers::IsSystemFontShorthand(valueID))
        {
          return css::keywords::Normal {};
        }

        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return css::keywords::Normal {};
    }
  }

  Ref<css::Value> CSSValueCreation<FontStyle>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                          const FontStyle &value) noexcept
  {
    if (!value.PlatformSlope() || !*value.PlatformSlope())
    {
      return CreateCSSValue(pool, style, css::keywords::Normal {});
    }

    if (*value.PlatformSlope() == ItalicValue())
    {
      if (value.PlatformAxis() == FontStyleAxis::ital)
      {
        return CreateCSSValue(pool, style, css::keywords::Italic {});
      }
      return CreateCSSValue(pool, style, css::keywords::Oblique {});
    }

    return css::FontStyleWithAngleValue::Create(
      ConvertToCSS(FontStyle::Angle {static_cast<float>(*value.PlatformSlope())}, style));
  }

  // MARK: - Blending

  bool Blending<FontStyle>::CanBlend(const FontStyle &a, const FontStyle &b) noexcept
  {
    return a.PlatformAxis() == FontStyleAxis::slnt && b.PlatformAxis() == FontStyleAxis::slnt;
  }

  FontStyle Blending<FontStyle>::Blend(const FontStyle &a, const FontStyle &b,
                                       const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      return context.Progress < 0.5 ? a : b;
    }

    if (!a.PlatformSlope() && !b.PlatformSlope())
    {
      return css::keywords::Normal {};
    }

    return style::Blend(a.GetAngle().value_or(FontStyle::Angle {0}),
                        b.GetAngle().value_or(FontStyle::Angle {0}), context);
  }
}

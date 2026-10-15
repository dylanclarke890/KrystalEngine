#include "Krystal.Booey/Style/Values/TextDecoration/TextDecorationThickness.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/FontMetrics.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"

namespace krys::boo::style
{
  static constexpr float textDecorationBaseFontSize = 16;

  float TextDecorationThickness::Resolve(const RenderStyle &style) const noexcept
  {
    return krys::SwitchOn(
      _value, [&](const css::keywords::Auto &)
      { return style.ComputedFontSize() / textDecorationBaseFontSize; }, [&](const css::keywords::FromFont &)
      { return style.MetricsOfPrimaryFont().UnderlineThickness().value_or(0); },
      [&](const TextDecorationThicknessLength &length)
      { return style::Evaluate<float>(length, style.ComputedFontSize(), style.UsedZoomForLength()); });
  }

  // MARK: - Conversion

  TextDecorationThickness
    CSSValueConversion<TextDecorationThickness>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Auto {};
    }

    if (primitiveValue->IsValueId())
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Auto:     return css::keywords::Auto {};
        case css::ValueId::FromFont: return css::keywords::FromFont {};
        default:                     break;
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::Auto {};
    }

    return ConvertCSSValueToStyle<TextDecorationThicknessLength>(state, *primitiveValue);
  }

  // MARK: - Blending

  bool Blending<TextDecorationThickness>::CanBlend(const TextDecorationThickness &a,
                                                   const TextDecorationThickness &b,
                                                   const RenderStyle &aStyle,
                                                   const RenderStyle &bStyle) noexcept
  {
    if (a.IsAuto() || b.IsAuto())
    {
      return false;
    }
    return a.Resolve(aStyle) != b.Resolve(bStyle);
  }

  TextDecorationThickness Blending<TextDecorationThickness>::Blend(const TextDecorationThickness &a,
                                                                   const TextDecorationThickness &b,
                                                                   const RenderStyle &aStyle,
                                                                   const RenderStyle &bStyle,
                                                                   const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? b : a;
    }

    return TextDecorationThicknessLength {
      TextDecorationThicknessLength::Fixed {boo::Blend(a.Resolve(aStyle), b.Resolve(bStyle), context)}};
  }
}

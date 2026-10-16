#include "Krystal.Booey/Style/Values/TextDecoration/TextShadow.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

#include "Krystal.Booey/CSS/Values/TextShadowPropertyValue.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Borders/ShadowInterpolation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <ranges>

namespace krys::boo::style
{
  // MARK: - Conversion

  css::TextShadow ToCSS<TextShadow>::operator()(const TextShadow &value, const RenderStyle &style) noexcept
  {
    return {
      .Color = ConvertToCSS(value.Color, style),
      .Location = ConvertToCSS(value.Location, style),
      .Blur = ConvertToCSS(value.Blur, style),
    };
  }

  TextShadow ToStyle<css::TextShadow>::operator()(const css::TextShadow &value,
                                                  const BuilderState &state) noexcept
  {
    return {
      .Color = value.Color ? ConvertToStyle(*value.Color, state) : Color::GetCurrentColor(),
      .Location = ConvertToStyle(value.Location, state),
      .Blur = value.Blur ? ConvertToStyle(*value.Blur, state) : Length<css::NonNegativeUnzoomed> {0},
    };
  }

  Ref<css::Value> CSSValueCreation<TextShadowList>::operator()(css::ValuePool &, const RenderStyle &style,
                                                               const TextShadowList &value) noexcept
  {
    css::TextShadowProperty::List list;

    for (const auto &shadow : value | std::views::reverse)
    {
      list.Value.push_back(ConvertToCSS(shadow, style));
    }

    return css::TextShadowPropertyValue::Create(css::TextShadowProperty {krys::move(list)});
  }

  TextShadows CSSValueConversion<TextShadows>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    if (value.ValueId() == css::ValueId::None)
    {
      return css::keywords::None {};
    }

    auto *shadow = RequiredDowncast<css::TextShadowPropertyValue>(state, value);
    if (!shadow)
    {
      return css::keywords::None {};
    }

    return krys::SwitchOn(
      shadow->Shadow(), [&](const css::keywords::None &) -> TextShadows { return css::keywords::None {}; },
      [&](const typename css::TextShadowProperty::List &list) -> TextShadows
      {
        return TextShadows::List::map(list | std::views::reverse, [&](const css::TextShadow &element)
                                      { return ConvertToStyle(element, state); });
      });
  }

  // MARK: - Serialization

  void Serialiser<TextShadowList>::operator()(StringBuilder &builder,
                                              const css::SerialisationContext &context,
                                              const RenderStyle &style, const TextShadowList &value) noexcept
  {
    SerialisationForCSSOnRangeLike(builder, context, style, value | std::views::reverse,
                                   css::SerialisationSeparatorString<TextShadowList>);
  }

  // MARK: - Blending

  TextShadow Blending<TextShadow>::Blend(const TextShadow &a, const TextShadow &b, const RenderStyle &aStyle,
                                         const RenderStyle &bStyle, const BlendingContext &context) noexcept
  {
    return {
      .Color = boo::Blend(aStyle.ColorResolvingCurrentColor(a.Color),
                          bStyle.ColorResolvingCurrentColor(b.Color), context),
      .Location = krys::boo::style::Blend(a.Location, b.Location, context),
      .Blur = krys::boo::style::Blend(a.Blur, b.Blur, context),
    };
  }

  struct MatchingTextShadows
  {
    static const TextShadow &ShadowForInterpolation(const TextShadow &)
    {
      static NeverDestroyed<const TextShadow> defaultShadowData {TextShadow {
        .Color = {boo::Color::transparentBlack},
        .Location = {{0}, {0}},
        .Blur = {0},
      }};
      return defaultShadowData.get();
    }
  };

  bool Blending<TextShadows>::CanBlend(const TextShadows &from, const TextShadows &to,
                                       CompositeOperation compositeOperation) noexcept
  {
    return ShadowInterpolation<TextShadows, MatchingTextShadows>::CanInterpolate(from, to,
                                                                                 compositeOperation);
  }

  TextShadows Blending<TextShadows>::Blend(const TextShadows &from, const TextShadows &to,
                                           const RenderStyle &fromStyle, const RenderStyle &toStyle,
                                           const BlendingContext &context) noexcept
  {
    return ShadowInterpolation<TextShadows, MatchingTextShadows>::Interpolate(from, to, fromStyle, toStyle,
                                                                              context);
  }
}

#include "Krystal.Booey/Style/Values/Borders/BoxShadow.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

#include "Krystal.Booey/CSS/Values/BoxShadowPropertyValue.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
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

  css::BoxShadow ToCSS<BoxShadow>::operator()(const BoxShadow &value, const RenderStyle &style) noexcept
  {
    return {
      .Color = ConvertToCSS(value.Color, style),
      .Location = ConvertToCSS(value.Location, style),
      .Blur = ConvertToCSS(value.Blur, style),
      .Spread = ConvertToCSS(value.Spread, style),
      .Inset = ConvertToCSS(value.Inset, style),
      .IsWebkitBoxShadow = value.IsWebkitBoxShadow,
    };
  }

  BoxShadow ToStyle<css::BoxShadow>::operator()(const css::BoxShadow &value,
                                                const BuilderState &state) noexcept
  {
    return {
      .Color = value.Color ? ConvertToStyle(*value.Color, state) : Color::GetCurrentColor(),
      .Location = ConvertToStyle(value.Location, state),
      .Blur = value.Blur ? ConvertToStyle(*value.Blur, state) : 0_css_px,
      .Spread = value.Spread ? ConvertToStyle(*value.Spread, state) : 0_css_px,
      .Inset = ConvertToStyle(value.Inset, state),
      .IsWebkitBoxShadow = value.IsWebkitBoxShadow,
    };
  }

  Ref<css::Value> CSSValueCreation<BoxShadowList>::operator()(css::ValuePool &, const RenderStyle &style,
                                                              const BoxShadowList &value) noexcept
  {
    css::BoxShadowProperty::List list;

    for (const auto &shadow : value | std::views::reverse)
    {
      list.Value.push_back(ConvertToCSS(shadow, style));
    }

    return css::BoxShadowPropertyValue::Create(css::BoxShadowProperty {krys::move(list)});
  }

  BoxShadows CSSValueConversion<BoxShadows>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (value.ValueId() == css::ValueId::None)
    {
      return css::keywords::None {};
    }

    auto *shadow = RequiredDowncast<css::BoxShadowPropertyValue>(state, value);
    if (!shadow)
    {
      return css::keywords::None {};
    }

    return krys::SwitchOn(
      shadow->Shadow(), [&](const css::keywords::None &) -> BoxShadows { return css::keywords::None {}; },
      [&](const typename css::BoxShadowProperty::List &list) -> BoxShadows
      {
        return BoxShadows::List::map(list | std::views::reverse, [&](const css::BoxShadow &element)
                                     { return ConvertToStyle(element, state); });
      });
  }

  // MARK: - Serialization

  void Serialiser<BoxShadowList>::operator()(StringBuilder &builder, const css::SerialisationContext &context,
                                             const RenderStyle &style, const BoxShadowList &value) noexcept
  {
    SerialisationForCSSOnRangeLike(builder, context, style, value | std::views::reverse,
                                   css::SerialisationSeparatorString<BoxShadowList>);
  }

  // MARK: - Blending

  KRYS_NODISCARD static inline Maybe<css::keywords::Inset> BlendInset(Maybe<css::keywords::Inset> a,
                                                                      Maybe<css::keywords::Inset> b,
                                                                      const BlendingContext &context) noexcept
  {
    if (a == b)
    {
      return b;
    }

    auto aVal = !a ? 1.0 : 0.0;
    auto bVal = !b ? 1.0 : 0.0;

    auto result = boo::Blend(aVal, bVal, context);
    return result > 0 ? null : Maybe(css::keywords::Inset {});
  }

  BoxShadow Blending<BoxShadow>::Blend(const BoxShadow &a, const BoxShadow &b, const RenderStyle &aStyle,
                                       const RenderStyle &bStyle, const BlendingContext &context) noexcept
  {
    return {.Color = boo::Blend(aStyle.ColorResolvingCurrentColor(a.Color),
                                bStyle.ColorResolvingCurrentColor(b.Color), context),
            .Location = krys::boo::style::Blend(a.Location, b.Location, context),
            .Blur = krys::boo::style::Blend(a.Blur, b.Blur, context),
            .Spread = krys::boo::style::Blend(a.Spread, b.Spread, context),
            .Inset = BlendInset(a.Inset, b.Inset, context),
            .IsWebkitBoxShadow = b.IsWebkitBoxShadow};
  }

  struct MatchingBoxShadows
  {
    static const BoxShadow &ShadowForInterpolation(const BoxShadow &shadowToMatch)
    {
      static NeverDestroyed<const BoxShadow> defaultShadowData {
        BoxShadow {.Color = {boo::Color::transparentBlack},
                   .Location = {{0}, {0}},
                   .Blur = {0},
                   .Spread = {0},
                   .Inset = null,
                   .IsWebkitBoxShadow = false}};
      static NeverDestroyed<const BoxShadow> defaultInsetShadowData {
        BoxShadow {.Color = {boo::Color::transparentBlack},
                   .Location = {{0}, {0}},
                   .Blur = {0},
                   .Spread = {0},
                   .Inset = css::keywords::Inset {},
                   .IsWebkitBoxShadow = false}};
      static NeverDestroyed<const BoxShadow> defaultWebKitBoxShadowData {
        BoxShadow {.Color = {boo::Color::transparentBlack},
                   .Location = {{0}, {0}},
                   .Blur = {0},
                   .Spread = {0},
                   .Inset = null,
                   .IsWebkitBoxShadow = true}};
      static NeverDestroyed<const BoxShadow> defaultInsetWebKitBoxShadowData {
        BoxShadow {.Color = {boo::Color::transparentBlack},
                   .Location = {{0}, {0}},
                   .Blur = {0},
                   .Spread = {0},
                   .Inset = css::keywords::Inset {},
                   .IsWebkitBoxShadow = true}};

      if (IsInset(shadowToMatch))
      {
        return shadowToMatch.IsWebkitBoxShadow ? defaultInsetWebKitBoxShadowData.get()
                                               : defaultInsetShadowData.get();
      }
      else
      {
        return shadowToMatch.IsWebkitBoxShadow ? defaultWebKitBoxShadowData.get() : defaultShadowData.get();
      }
    }
  };

  bool Blending<BoxShadows>::CanBlend(const BoxShadows &from, const BoxShadows &to,
                                      CompositeOperation compositeOperation) noexcept
  {
    return ShadowInterpolation<BoxShadows, MatchingBoxShadows>::CanInterpolate(from, to, compositeOperation);
  }

  BoxShadows Blending<BoxShadows>::Blend(const BoxShadows &from, const BoxShadows &to,
                                         const RenderStyle &fromStyle, const RenderStyle &toStyle,
                                         const BlendingContext &context) noexcept
  {
    return ShadowInterpolation<BoxShadows, MatchingBoxShadows>::Interpolate(from, to, fromStyle, toStyle,
                                                                            context);
  }
}

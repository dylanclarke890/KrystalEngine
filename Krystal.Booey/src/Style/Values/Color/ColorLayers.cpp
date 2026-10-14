#include "Krystal.Booey/Style/Values/Color/ColorLayers.hpp"
#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorLayersResolver.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorResolutionState.hpp"
#include <algorithm>

namespace krys::boo::style
{
  namespace
  {
    KRYS_NODISCARD List<boo::Color> ResolveLayers(const ColorLayers &colorLayers,
                                                  const boo::Color &currentColor) noexcept
    {
      // FIXME: This should be made into a lazy transformed range to avoid the unnecessary temporary
      // allocation.
      List<boo::Color> colors;
      colors.reserve(colorLayers.Colors.size());
      for (const auto &color : colorLayers.Colors)
      {
        colors.push_back(color.ResolveColor(currentColor));
      }
      return colors;
    }
  }

  // MARK: - Conversion

  Color ToStyleColor(const css::ColorLayers &unresolved, ColorResolutionState &state) noexcept
  {
    ColorResolutionStateNester nester {state};

    auto colors = css::CommaSeparatedList<Color>::map(unresolved.Colors, [&](const auto &color) -> Color
                                                      { return ToStyleColor(color, state); });

    if (!std::ranges::all_of(colors, [](const auto &color) { return color.IsResolvedColor(); }))
    {
      // If the any of the layer's colors are not resolved, we cannot fully resolve the
      // color yet. Instead we return a style::ColorLayers to be resolved at use time.
      return Color {ColorLayers {.BlendMode = unresolved.BlendMode, .Colors = krys::move(colors)}};
    }

    // FIXME: This should be made into a lazy transformed range to avoid the unnecessary temporary allocation.
    List<boo::Color> resolvedColors;
    resolvedColors.reserve(colors.size());
    for (const auto &color : colors)
    {
      resolvedColors.push_back(color.GetResolvedColor());
    }

    return css::BlendSourceOver(
      css::ColorLayersResolver {.BlendMode = unresolved.BlendMode, .Colors = krys::move(resolvedColors)});
  }

  // MARK: Resolve

  boo::Color ResolveColor(const ColorLayers &colorLayers, const boo::Color &currentColor) noexcept
  {
    return css::BlendSourceOver(css::ColorLayersResolver {
      .BlendMode = colorLayers.BlendMode, .Colors = ResolveLayers(colorLayers, currentColor)});
  }

  // MARK: - Current Color

  bool ContainsCurrentColor(const ColorLayers &colorLayers) noexcept
  {
    return std::ranges::any_of(colorLayers.Colors,
                               [&](const auto &color) { return style::ContainsCurrentColor(color); });
  }

  // MARK: - Serialisation

  void SerialisationForCSSTokenization(css::CSSOMString &builder, const css::SerialisationContext &context,
                                       const ColorLayers &value) noexcept
  {
    builder.append(u8"color-layers(");

    if (value.BlendMode != BlendMode::Normal)
    {
      builder.append(css::ToLower(ToValueId(value.BlendMode)));
      builder.append(u8", ");
    }

    auto first = true;
    for (const auto &color : value.Colors)
    {
      if (!first)
      {
        builder.append(u8", ");
      }
      first = false;
      SerialisationForCSSTokenization(builder, context, color);
    }

    builder.push_back(u8')');
  }

  css::CSSOMString SerialisationForCSSTokenization(const css::SerialisationContext &context,
                                                   const ColorLayers &colorLayers) noexcept
  {
    css::CSSOMString builder;
    SerialisationForCSSTokenization(builder, context, colorLayers);
    return builder;
  }
}

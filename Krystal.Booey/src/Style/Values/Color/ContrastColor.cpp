#include "Krystal.Booey/Style/Values/Color/ContrastColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/ContrastColorResolver.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorResolutionState.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  Color ToStyleColor(const css::ContrastColor &unresolved, ColorResolutionState &state) noexcept
  {
    ColorResolutionStateNester nester {state};

    auto color = ToStyleColor(unresolved.Color, state);
    if (!color.IsResolvedColor())
    {
      return Color {ContrastColor {krys::move(color)}};
    }

    return css::Resolve(css::ContrastColorResolver {color.GetResolvedColor()});
  }

  // MARK: - Resolve

  boo::Color ResolveColor(const ContrastColor &contrastColor, const boo::Color &currentColor) noexcept
  {
    return css::Resolve(css::ContrastColorResolver {contrastColor.Color.ResolveColor(currentColor)});
  }

  // MARK: - Current Color

  bool ContainsCurrentColor(const ContrastColor &contrastColor) noexcept
  {
    return style::ContainsCurrentColor(contrastColor.Color);
  }

  // MARK: - Serialisation

  void SerialisationForCSSTokenization(css::CSSOMString &builder, const css::SerialisationContext &context,
                                       const ContrastColor &contrastColor) noexcept
  {
    builder.append(u8"contrast-color(");
    SerialisationForCSSTokenization(builder, context, contrastColor.Color);
    builder.push_back(u8')');
  }

  css::CSSOMString SerialisationForCSSTokenization(const css::SerialisationContext &context,
                                                   const ContrastColor &contrastColor) noexcept
  {
    css::CSSOMString builder;
    SerialisationForCSSTokenization(builder, context, contrastColor);
    return builder;
  }
}

#pragma once

#include "Krystal.Booey/CSS/Values/Color/ColorLayers.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  struct ColorResolutionState;

  /// @brief A `color-layers()` that couldn't be resolved during style building because a layer depends on
  /// `currentcolor`.
  struct ColorLayers
  {
    boo::BlendMode BlendMode;
    css::CommaSeparatedList<Color> Colors;

    KRYS_NODISCARD bool operator==(const ColorLayers &) const noexcept = default;
  };

  KRYS_NODISCARD Color ToStyleColor(const css::ColorLayers &unresolved, ColorResolutionState &state) noexcept;
  KRYS_NODISCARD boo::Color ResolveColor(const ColorLayers &colorLayers,
                                         const boo::Color &currentColor) noexcept;
  KRYS_NODISCARD bool ContainsCurrentColor(const ColorLayers &colorLayers) noexcept;

  void SerialisationForCSSTokenization(StringBuilder &builder, const css::SerialisationContext &context,
                                       const ColorLayers &value) noexcept;
  KRYS_NODISCARD css::CSSOMString SerialisationForCSSTokenization(const css::SerialisationContext &context,
                                                                  const ColorLayers &value) noexcept;
}

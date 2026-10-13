#pragma once

#include "Krystal.Booey/CSS/Values/Color/ContrastColor.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"

namespace krys::boo::style
{
  struct ColorResolutionState;

  /// @brief A `contrast-color()` that couldn't be resolved during style building because its colour depends
  /// on `currentcolor`.
  struct ContrastColor
  {
    style::Color Color;

    KRYS_NODISCARD bool operator==(const ContrastColor &) const noexcept = default;
  };

  KRYS_NODISCARD Color ToStyleColor(const css::ContrastColor &unresolved,
                                    ColorResolutionState &state) noexcept;
  KRYS_NODISCARD boo::Color ResolveColor(const ContrastColor &contrastColor,
                                         const boo::Color &currentColor) noexcept;
  KRYS_NODISCARD bool ContainsCurrentColor(const ContrastColor &contrastColor) noexcept;

  void SerialisationForCSSTokenization(css::CSSOMString &builder, const css::SerialisationContext &context,
                                       const ContrastColor &value) noexcept;
  KRYS_NODISCARD css::CSSOMString SerialisationForCSSTokenization(const css::SerialisationContext &context,
                                                                  const ContrastColor &value) noexcept;
}

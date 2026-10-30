#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <string_view>

namespace krys
{
  /// @brief How a colour is mixed with the colour beneath it before compositing, as CSS's `mix-blend-mode`
  /// and `background-blend-mode` name them.
  enum class BlendMode : uint8
  {
    Normal = 1, // Start with 1 to match SVG's blendmode enumeration.
    Multiply,
    Screen,
    Darken,
    Lighten,
    Overlay,
    ColorDodge,
    ColorBurn,
    HardLight,
    SoftLight,
    Difference,
    Exclusion,
    Hue,
    Saturation,
    Color,
    Luminosity,
    PlusDarker,
    PlusLighter
  };

  /// @brief The CSS name of @p blendMode, such as `color-dodge`.
  KRYS_NODISCARD constexpr std::string_view BlendModeName(BlendMode blendMode) noexcept
  {
    constexpr std::string_view blendOperatorNames[] = {
      "normal",      "multiply",   "screen",     "darken",     "lighten",     "overlay",
      "color-dodge", "color-burn", "hard-light", "soft-light", "difference",  "exclusion",
      "hue",         "saturation", "color",      "luminosity", "plus-darker", "plus-lighter"};
    return blendOperatorNames[static_cast<uint32>(blendMode) - static_cast<uint32>(BlendMode::Normal)];
  }
}

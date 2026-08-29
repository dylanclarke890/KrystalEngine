#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class ColorType : uint8_t
  {
    Absolute = 0,     ///< Absolute color values (e.g., #RRGGBB, rgb(), hsl(), etc.)
    Current = 1 << 0, ///< The current color value (e.g., currentColor)
    System = 1 << 1   ///< System color values (e.g., ButtonFace, Highlight, etc.)
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::ColorType, 3uz);

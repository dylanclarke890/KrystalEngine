#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class SVGElementFlags : uint8
  {
    None = 0,
    IsSvgGraphicsElement = 1 << 0,
    IsSvgGeometryElement = 1 << 1,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::SVGElementFlags, 3uz);
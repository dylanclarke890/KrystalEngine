#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::svg
{
  enum class SVGElementFlags : uint8
  {
    None = 0,
    IsSvgGraphicsElement = 1 << 0,
    IsSvgGeometryElement = 1 << 1,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::svg::SVGElementFlags, 3uz);
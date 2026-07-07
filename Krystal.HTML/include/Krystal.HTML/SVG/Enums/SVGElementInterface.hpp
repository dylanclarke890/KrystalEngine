#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class SVGElementInterface : uint8
  {
    None,
    Circle,
    Defs,
    Desc,
    Ellipse,
    ForeignObject,
    G,
    Graphics,
    Geometry,
    Image,
    Line,
    Metadata,
    Polygon,
    Polyline,
    Rect,
    Script,
    Switch,
    Style,
    Svg,
    Symbol,
    Title,
    Unknown,
    Use,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::SVGElementInterface, 23uz);
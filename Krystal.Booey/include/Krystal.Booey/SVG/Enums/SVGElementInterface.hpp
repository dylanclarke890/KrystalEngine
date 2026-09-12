#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::svg
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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::svg::SVGElementInterface, 23uz);
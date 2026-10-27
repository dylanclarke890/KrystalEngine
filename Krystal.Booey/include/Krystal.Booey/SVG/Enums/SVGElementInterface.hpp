#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::svg
{
  enum class SVGElementInterface : uint8
  {
    None,
    A,
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
    LinearGradient,
    Metadata,
    Path,
    Pattern,
    Polygon,
    Polyline,
    RadialGradient,
    Rect,
    Script,
    Stop,
    Switch,
    Style,
    Svg,
    Symbol,
    Title,
    Unknown,
    Use,
    View,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::svg::SVGElementInterface, 30uz);
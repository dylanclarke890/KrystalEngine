#pragma once

#include "Krystal.Booey/CSS/Values/Shapes/CircleFunction.hpp"
#include "Krystal.Booey/CSS/Values/Shapes/EllipseFunction.hpp"
#include "Krystal.Booey/CSS/Values/Shapes/InsetFunction.hpp"
#include "Krystal.Booey/CSS/Values/Shapes/PathFunction.hpp"
#include "Krystal.Booey/CSS/Values/Shapes/PolygonFunction.hpp"
#include "Krystal.Booey/CSS/Values/Shapes/RectFunction.hpp"
#include "Krystal.Booey/CSS/Values/Shapes/ShapeFunction.hpp"
#include "Krystal.Booey/CSS/Values/Shapes/XywhFunction.hpp"

namespace krys::boo::css
{
  // <basic-shape> = <circle()> | <ellipse() | <inset()> | <path()> | <polygon()> | <rect()> | <shape()>
  //               | <xywh()>
  // https://drafts.csswg.org/css-shapes/#typedef-basic-shape
  using BasicShape = Variant<CircleFunction, EllipseFunction, InsetFunction, PathFunction, PolygonFunction,
                             RectFunction, ShapeFunction, XywhFunction>;
}

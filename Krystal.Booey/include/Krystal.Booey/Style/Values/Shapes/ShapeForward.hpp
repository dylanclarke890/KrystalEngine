#pragma once

#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo
{
  enum css::ValueId : uint16;
  template <css::ValueId C, typename T>
  struct css::FunctionNotation;

  namespace style
  {
    struct Circle;
    struct Ellipse;
    struct Inset;
    struct Path;
    struct Polygon;
    struct Shape;

    using CircleFunction = css::FunctionNotation<css::ValueId::Circle, Circle>;
    using EllipseFunction = css::FunctionNotation<css::ValueId::Ellipse, Ellipse>;
    using InsetFunction = css::FunctionNotation<css::ValueId::Inset, Inset>;
    using PathFunction = css::FunctionNotation<css::ValueId::Path, Path>;
    using PolygonFunction = css::FunctionNotation<css::ValueId::Polygon, Polygon>;
    using ShapeFunction = css::FunctionNotation<css::ValueId::Shape, Shape>;

    using BasicShape =
      Variant<CircleFunction, EllipseFunction, InsetFunction, PathFunction, PolygonFunction, ShapeFunction>;

  }
}

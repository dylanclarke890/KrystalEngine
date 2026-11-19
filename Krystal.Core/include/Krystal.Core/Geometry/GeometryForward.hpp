#pragma once

#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys
{
  template <Number T>
  class Point;

  class PolygonF;

  class QuadF;

  template <Number T>
  class Rect;

  class Region;

  class SegmentF;

  template <Number T>
  class Size;

  using PointD = Point<double>;
  using PointF = Point<float>;
  using PointI = Point<int32>;

  using RectD = Rect<double>;
  using RectF = Rect<float>;
  using RectI = Rect<int32>;

  using SizeD = Size<double>;
  using SizeF = Size<float>;
  using SizeI = Size<int32>;
}

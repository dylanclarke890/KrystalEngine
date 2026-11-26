#pragma once

#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys
{
  /// @brief What the geometry types need to know about a coordinate type that isn't a built-in number;
  /// specialise it for one (as Krystal.Web does for its fixed-point `LayoutUnit`).
  /// @note A specialisation may also define `InfiniteRectMin()` and `InfiniteRectExtent()`, the location and
  /// size of the type's `Rect::InfiniteRect()`, which `Rect::Scale` then leaves alone.
  template <typename T>
  struct GeometryTraits
  {
    /// @brief Whether `T` is a fixed-point number. Like an integer, a point is in a rect when the 1x1 square
    /// below and to the right of it is, and moving a maximum edge clamps the size at zero; unlike one,
    /// scaling keeps the fraction. Integers convert to it implicitly, and it converts implicitly to floating
    /// point.
    constexpr static bool IsFixedPoint = false;
  };

  /// @brief A type geometry can be measured in: a built-in number, or a fixed-point type marked by
  /// `GeometryTraits`.
  template <typename T>
  concept GeometryCoordinate = Number<T> || GeometryTraits<T>::IsFixedPoint;

  template <GeometryCoordinate T>
  class Point;

  class Point3F;

  class PointGraphF;

  class PolygonF;

  class QuadF;

  template <GeometryCoordinate T>
  class Rect;

  class Region;

  struct SegmentF;

  template <GeometryCoordinate T>
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

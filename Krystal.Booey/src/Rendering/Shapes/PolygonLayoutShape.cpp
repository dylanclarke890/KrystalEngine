#include "Krystal.Booey/Rendering/Shapes/PolygonLayoutShape.hpp"
#include <cmath>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD FloatSize InwardEdgeNormal(const FloatPolygonEdge &edge) noexcept
    {
      FloatSize edgeDelta = edge.Vertex2() - edge.Vertex1();
      if (!edgeDelta.Width())
      {
        return FloatSize((edgeDelta.Height() > 0 ? -1.f : 1.f), 0);
      }
      if (!edgeDelta.Height())
      {
        return FloatSize(0, (edgeDelta.Width() > 0 ? 1.f : -1.f));
      }
      float edgeLength = edgeDelta.DiagonalLength();
      return FloatSize(-edgeDelta.Height() / edgeLength, edgeDelta.Width() / edgeLength);
    }

    KRYS_NODISCARD FloatSize OutwardEdgeNormal(const FloatPolygonEdge &edge) noexcept
    {
      return -InwardEdgeNormal(edge);
    }

    KRYS_NODISCARD float CircleXIntercept(float y, float radius) noexcept
    {
      krys_debug_assert(radius > 0);
      return radius * std::sqrt(1 - (y * y) / (radius * radius));
    }

    KRYS_NODISCARD FloatShapeInterval ClippedCircleXRange(const FloatPoint &center, float radius, float y1,
                                                          float y2) noexcept
    {
      if (y1 >= center.Y() + radius || y2 <= center.Y() - radius)
      {
        return FloatShapeInterval();
      }

      if (center.Y() >= y1 && center.Y() <= y2)
      {
        return FloatShapeInterval(center.X() - radius, center.X() + radius);
      }

      // Clip the circle to the vertical range y1,y2 and return the extent of the clipped circle's
      // projection on the X axis

      float xi = CircleXIntercept((y2 < center.Y() ? y2 : y1) - center.Y(), radius);
      return FloatShapeInterval(center.X() - xi, center.X() + xi);
    }
  }

  float OffsetPolygonEdge::XIntercept(float y) const noexcept
  {
    krys_debug_assert(y >= MinY() && y <= MaxY());

    if (Vertex1().Y() == Vertex2().Y() || Vertex1().X() == Vertex2().X())
    {
      return MinX();
    }
    if (y == MinY())
    {
      return Vertex1().Y() < Vertex2().Y() ? Vertex1().X() : Vertex2().X();
    }
    if (y == MaxY())
    {
      return Vertex1().Y() > Vertex2().Y() ? Vertex1().X() : Vertex2().X();
    }

    return Vertex1().X()
           + ((y - Vertex1().Y()) * (Vertex2().X() - Vertex1().X()) / (Vertex2().Y() - Vertex1().Y()));
  }

  FloatShapeInterval OffsetPolygonEdge::ClippedEdgeXRange(float y1, float y2) const noexcept
  {
    if (!OverlapsYRange(y1, y2) || (y1 == MaxY() && MinY() <= y1) || (y2 == MinY() && MaxY() >= y2))
    {
      return FloatShapeInterval();
    }

    if (IsWithinYRange(y1, y2))
    {
      return FloatShapeInterval(MinX(), MaxX());
    }

    // Clip the edge line segment to the vertical range y1,y2 and then return
    // the clipped line segment's horizontal range.

    FloatPoint minYVertex;
    FloatPoint maxYVertex;
    if (Vertex1().Y() < Vertex2().Y())
    {
      minYVertex = Vertex1();
      maxYVertex = Vertex2();
    }
    else
    {
      minYVertex = Vertex2();
      maxYVertex = Vertex1();
    }
    float xForY1 = (minYVertex.Y() < y1) ? XIntercept(y1) : minYVertex.X();
    float xForY2 = (maxYVertex.Y() > y2) ? XIntercept(y2) : maxYVertex.X();
    return FloatShapeInterval(std::min(xForY1, xForY2), std::max(xForY1, xForY2));
  }

  LayoutRect PolygonLayoutShape::ShapeMarginLogicalBoundingBox() const noexcept
  {
    FloatRect box = _polygon.BoundingBox();
    box.Inflate(ShapeMargin());
    return LayoutRect(box);
  }

  LineSegment PolygonLayoutShape::GetExcludedInterval(LayoutUnit logicalTop,
                                                      LayoutUnit logicalHeight) const noexcept
  {
    float y1 = logicalTop.ToFloat();
    float y2 = (logicalTop + logicalHeight).ToFloat();

    if (_polygon.IsEmpty() || !_polygon.BoundingBox().OverlapsYRange(y1 - ShapeMargin(), y2 + ShapeMargin()))
    {
      return {};
    }

    FloatShapeInterval excludedInterval;
    for (const FloatPolygonEdge &edge : _polygon.OverlappingEdges(y1 - ShapeMargin(), y2 + ShapeMargin()))
    {
      if (edge.MaxY() == edge.MinY())
      {
        continue;
      }
      if (!ShapeMargin())
      {
        excludedInterval.Unite(OffsetPolygonEdge(edge, FloatSize()).ClippedEdgeXRange(y1, y2));
      }
      else
      {
        excludedInterval.Unite(
          OffsetPolygonEdge(edge, OutwardEdgeNormal(edge) * ShapeMargin()).ClippedEdgeXRange(y1, y2));
        excludedInterval.Unite(
          OffsetPolygonEdge(edge, InwardEdgeNormal(edge) * ShapeMargin()).ClippedEdgeXRange(y1, y2));
        excludedInterval.Unite(ClippedCircleXRange(edge.Vertex1(), ShapeMargin(), y1, y2));
        excludedInterval.Unite(ClippedCircleXRange(edge.Vertex2(), ShapeMargin(), y1, y2));
      }
    }

    if (excludedInterval.IsEmpty())
    {
      return {};
    }

    if (ShouldFlipStartAndEndPoints(GetWritingMode()))
    {
      return {std::max(0.f, _boxLogicalWidth - excludedInterval.X2()),
              std::max(0.f, _boxLogicalWidth - excludedInterval.X1())};
    }

    return {excludedInterval.X1(), excludedInterval.X2()};
  }
}

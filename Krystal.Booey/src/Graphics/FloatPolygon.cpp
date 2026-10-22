#include "Krystal.Booey/Graphics/FloatPolygon.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool AreCollinearPoints(const FloatPoint &p0, const FloatPoint &p1,
                                           const FloatPoint &p2) noexcept
    {
      return !Determinant(p1 - p0, p2 - p0);
    }

    KRYS_NODISCARD bool AreCoincidentPoints(const FloatPoint &p0, const FloatPoint &p1) noexcept
    {
      return p0.X() == p1.X() && p0.Y() == p1.Y();
    }

    KRYS_NODISCARD uint32 NextVertexIndex(uint32 vertexIndex, uint32 nVertices, bool clockwise) noexcept
    {
      return ((clockwise) ? vertexIndex + 1 : vertexIndex - 1 + nVertices) % nVertices;
    }

    KRYS_NODISCARD uint32 FindNextEdgeVertexIndex(const FloatPolygon &polygon, uint32 vertexIndex1,
                                                  bool clockwise) noexcept
    {
      uint32 nVertices = polygon.NumberOfVertices();
      uint32 vertexIndex2 = NextVertexIndex(vertexIndex1, nVertices, clockwise);

      while (vertexIndex2
             && AreCoincidentPoints(polygon.VertexAt(vertexIndex1), polygon.VertexAt(vertexIndex2)))
      {
        vertexIndex2 = NextVertexIndex(vertexIndex2, nVertices, clockwise);
      }

      while (vertexIndex2)
      {
        uint32 vertexIndex3 = NextVertexIndex(vertexIndex2, nVertices, clockwise);
        if (!AreCollinearPoints(polygon.VertexAt(vertexIndex1), polygon.VertexAt(vertexIndex2),
                                polygon.VertexAt(vertexIndex3)))
        {
          break;
        }
        vertexIndex2 = vertexIndex3;
      }

      return vertexIndex2;
    }
  }

  FloatPolygon::FloatPolygon(List<FloatPoint> &&vertices) noexcept
      : _vertices(krys::move(vertices)), _empty(_vertices.size() < 3), _edges(_vertices.size())
  {
    uint32 nVertices = NumberOfVertices();
    if (nVertices)
    {
      _boundingBox.SetLocation(VertexAt(0));
    }

    if (_empty)
    {
      return;
    }

    uint32 minVertexIndex = 0;
    for (uint32 i = 1; i < nVertices; ++i)
    {
      const FloatPoint &vertex = VertexAt(i);
      if (vertex.Y() < VertexAt(minVertexIndex).Y()
          || (vertex.Y() == VertexAt(minVertexIndex).Y() && vertex.X() < VertexAt(minVertexIndex).X()))
      {
        minVertexIndex = i;
      }
    }
    FloatPoint nextVertex = VertexAt((minVertexIndex + 1) % nVertices);
    FloatPoint prevVertex = VertexAt((minVertexIndex + nVertices - 1) % nVertices);
    bool clockwise = Determinant(VertexAt(minVertexIndex) - prevVertex, nextVertex - prevVertex) > 0;

    uint32 edgeIndex = 0;
    uint32 vertexIndex1 = 0;
    do
    {
      _boundingBox.Extend(VertexAt(vertexIndex1));
      uint32 vertexIndex2 = FindNextEdgeVertexIndex(*this, vertexIndex1, clockwise);
      _edges[edgeIndex]._polygon = this;
      _edges[edgeIndex]._vertexIndex1 = vertexIndex1;
      _edges[edgeIndex]._vertexIndex2 = vertexIndex2;
      _edges[edgeIndex]._edgeIndex = edgeIndex;
      ++edgeIndex;
      vertexIndex1 = vertexIndex2;
    } while (vertexIndex1);

    if (edgeIndex > 3)
    {
      const FloatPolygonEdge &firstEdge = _edges[0];
      const FloatPolygonEdge &lastEdge = _edges[edgeIndex - 1];
      if (AreCollinearPoints(lastEdge.Vertex1(), lastEdge.Vertex2(), firstEdge.Vertex2()))
      {
        _edges[0]._vertexIndex1 = lastEdge._vertexIndex1;
        edgeIndex--;
      }
    }

    _edges.resize(edgeIndex);
    _empty = _edges.size() < 3;

    if (_empty)
    {
      return;
    }

    for (auto &edge : _edges)
    {
      _edgeTree.add({edge.MinY(), edge.MaxY(), &edge});
    }
  }

  List<std::reference_wrapper<const FloatPolygonEdge>>
    FloatPolygon::OverlappingEdges(float minY, float maxY) const noexcept
  {
    auto overlappingEdgeIntervals = _edgeTree.allOverlaps({minY, maxY});
    List<std::reference_wrapper<const FloatPolygonEdge>> edges;
    edges.reserve(overlappingEdgeIntervals.size());
    for (const auto &interval : overlappingEdgeIntervals)
    {
      edges.emplace_back(*interval.Data());
    }
    return edges;
  }

  bool VertexPair::Intersection(const VertexPair &other, FloatPoint &point) const noexcept
  {
    // See: http://paulbourke.net/geometry/pointlineplane/, "Intersection point of two lines in 2 dimensions"

    const FloatSize &thisDelta = Vertex2() - Vertex1();
    const FloatSize &otherDelta = other.Vertex2() - other.Vertex1();
    float denominator = Determinant(thisDelta, otherDelta);
    if (!denominator)
    {
      return false;
    }

    // The two line segments: "this" vertex1,vertex2 and "other" vertex1,vertex2, have been defined
    // in parametric form. Each point on the line segment is: vertex1 + u * (vertex2 - vertex1),
    // when 0 <= u <= 1. We're computing the values of u for each line at their intersection point.

    const FloatSize &vertex1Delta = Vertex1() - other.Vertex1();
    float uThisLine = Determinant(otherDelta, vertex1Delta) / denominator;
    float uOtherLine = Determinant(thisDelta, vertex1Delta) / denominator;

    if (uThisLine < 0 || uOtherLine < 0 || uThisLine > 1 || uOtherLine > 1)
    {
      return false;
    }

    point = Vertex1() + uThisLine * thisDelta;
    return true;
  }
}

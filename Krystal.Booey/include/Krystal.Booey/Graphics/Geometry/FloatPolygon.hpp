/*
 * Copyright (C) 2012 Adobe Systems Incorporated. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/IntervalTree.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <functional>

namespace krys::boo
{
  class FloatPolygonEdge;

  using EdgeIntervalTree = IntervalTree<float, FloatPolygonEdge *>;

  /// @brief A pair of vertices: a line segment.
  class VertexPair
  {
  public:
    virtual ~VertexPair() noexcept = default;

    KRYS_NODISCARD virtual const FloatPoint &Vertex1() const noexcept = 0;
    KRYS_NODISCARD virtual const FloatPoint &Vertex2() const noexcept = 0;

    KRYS_NODISCARD float MinX() const noexcept
    {
      return std::min(Vertex1().X(), Vertex2().X());
    }
    KRYS_NODISCARD float MinY() const noexcept
    {
      return std::min(Vertex1().Y(), Vertex2().Y());
    }
    KRYS_NODISCARD float MaxX() const noexcept
    {
      return std::max(Vertex1().X(), Vertex2().X());
    }
    KRYS_NODISCARD float MaxY() const noexcept
    {
      return std::max(Vertex1().Y(), Vertex2().Y());
    }

    /// @brief Whether this segment intersects @p other, and if so, where.
    KRYS_NODISCARD bool Intersection(const VertexPair &other, FloatPoint &point) const noexcept;
  };

  /// @brief A polygon, with its edges indexed by their vertical range. Collinear and coincident vertices are
  /// merged into single edges.
  class FloatPolygon
  {
    // The edges point back to the polygon, and the edge tree points to the edges.
    KRYS_NON_COPY_MOVABLE(FloatPolygon);

  private:
    List<FloatPoint> _vertices;
    FloatRect _boundingBox;
    bool _empty;
    List<FloatPolygonEdge> _edges;
    EdgeIntervalTree _edgeTree; // Each EdgeIntervalTree node stores minY, maxY, and a ("UserData") pointer to
                                // a FloatPolygonEdge.

  public:
    explicit FloatPolygon(List<FloatPoint> &&vertices) noexcept;

    KRYS_NODISCARD const FloatPoint &VertexAt(uint32 index) const noexcept
    {
      return _vertices[index];
    }
    KRYS_NODISCARD uint32 NumberOfVertices() const noexcept
    {
      return static_cast<uint32>(_vertices.size());
    }

    KRYS_NODISCARD const FloatPolygonEdge &EdgeAt(uint32 index) const noexcept
    {
      return _edges[index];
    }
    KRYS_NODISCARD uint32 NumberOfEdges() const noexcept
    {
      return static_cast<uint32>(_edges.size());
    }

    KRYS_NODISCARD FloatRect BoundingBox() const noexcept
    {
      return _boundingBox;
    }
    /// @brief The edges whose vertical range overlaps `[minY, maxY]`.
    KRYS_NODISCARD List<std::reference_wrapper<const FloatPolygonEdge>>
      OverlappingEdges(float minY, float maxY) const noexcept;
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _empty;
    }
  };

  /// @brief An edge of a polygon.
  class FloatPolygonEdge : public VertexPair
  {
    friend class FloatPolygon;

  private:
    // Edge vertex index1 is less than index2, except the last edge, where index2 is 0. When a polygon edge
    // is defined by 3 or more colinear vertices, index2 can be the index of the last colinear vertex.
    uint32 _vertexIndex1 {0};
    uint32 _vertexIndex2 {0};
    uint32 _edgeIndex {0};
    const FloatPolygon *_polygon {nullptr};

  public:
    KRYS_NODISCARD const FloatPoint &Vertex1() const noexcept override
    {
      krys_debug_assert(_polygon);
      return _polygon->VertexAt(_vertexIndex1);
    }

    KRYS_NODISCARD const FloatPoint &Vertex2() const noexcept override
    {
      krys_debug_assert(_polygon);
      return _polygon->VertexAt(_vertexIndex2);
    }

    KRYS_NODISCARD const FloatPolygonEdge &PreviousEdge() const noexcept
    {
      krys_debug_assert(_polygon && _polygon->NumberOfEdges() > 1);
      return _polygon->EdgeAt((_edgeIndex + _polygon->NumberOfEdges() - 1) % _polygon->NumberOfEdges());
    }

    KRYS_NODISCARD const FloatPolygonEdge &NextEdge() const noexcept
    {
      krys_debug_assert(_polygon && _polygon->NumberOfEdges() > 1);
      return _polygon->EdgeAt((_edgeIndex + 1) % _polygon->NumberOfEdges());
    }

    KRYS_NODISCARD const FloatPolygon *Polygon() const noexcept
    {
      return _polygon;
    }
    KRYS_NODISCARD uint32 VertexIndex1() const noexcept
    {
      return _vertexIndex1;
    }
    KRYS_NODISCARD uint32 VertexIndex2() const noexcept
    {
      return _vertexIndex2;
    }
    KRYS_NODISCARD uint32 EdgeIndex() const noexcept
    {
      return _edgeIndex;
    }
  };
}

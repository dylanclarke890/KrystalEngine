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

#include "Krystal.Booey/Graphics/Geometry/FloatPolygon.hpp"
#include "Krystal.Booey/Rendering/Shapes/LayoutShape.hpp"
#include "Krystal.Booey/Rendering/Shapes/ShapeInterval.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo
{
  /// @brief A polygon edge moved by an offset.
  class OffsetPolygonEdge final : public VertexPair
  {
  private:
    FloatPoint _vertex1;
    FloatPoint _vertex2;

  public:
    OffsetPolygonEdge(const FloatPolygonEdge &edge, const FloatSize &offset) noexcept
        : _vertex1(edge.Vertex1() + offset), _vertex2(edge.Vertex2() + offset)
    {
    }

    KRYS_NODISCARD const FloatPoint &Vertex1() const noexcept override
    {
      return _vertex1;
    }
    KRYS_NODISCARD const FloatPoint &Vertex2() const noexcept override
    {
      return _vertex2;
    }

    KRYS_NODISCARD bool IsWithinYRange(float y1, float y2) const noexcept
    {
      return y1 <= MinY() && y2 >= MaxY();
    }
    KRYS_NODISCARD bool OverlapsYRange(float y1, float y2) const noexcept
    {
      return y2 >= MinY() && y1 <= MaxY();
    }
    KRYS_NODISCARD float XIntercept(float y) const noexcept;
    KRYS_NODISCARD FloatShapeInterval ClippedEdgeXRange(float y1, float y2) const noexcept;
  };

  /// @brief A `polygon()` shape.
  class PolygonLayoutShape : public LayoutShape
  {
    KRYS_NON_COPY_MOVABLE(PolygonLayoutShape);

  private:
    FloatPolygon _polygon;
    float _boxLogicalWidth {0.f};

  public:
    PolygonLayoutShape(List<FloatPoint> &&vertices, float boxLogicalWidth) noexcept
        : _polygon(krys::move(vertices)), _boxLogicalWidth(boxLogicalWidth)
    {
    }

    KRYS_NODISCARD LayoutRect ShapeMarginLogicalBoundingBox() const noexcept override;
    KRYS_NODISCARD bool IsEmpty() const noexcept override
    {
      return _polygon.IsEmpty();
    }
    KRYS_NODISCARD LineSegment GetExcludedInterval(LayoutUnit logicalTop,
                                                   LayoutUnit logicalHeight) const noexcept override;
  };
}

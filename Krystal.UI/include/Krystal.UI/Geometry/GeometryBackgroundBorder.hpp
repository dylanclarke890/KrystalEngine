#pragma once

#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Gfx/Resources/Mesh.hpp"
#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Maths.hpp"
#include "Krystal.UI/Geometry/BorderMetrics.hpp"

namespace Krys::UI
{
  class GeometryBackgroundBorder
  {
    NO_COPY_MOVE(GeometryBackgroundBorder)

    using Vertex = Gfx::Vertex::Position2D_ColourbPremultiplied_UV;
    using Index = uint32;

  private:
    Gfx::MeshData &_data;

  public:
    GeometryBackgroundBorder(Gfx::MeshData &data) noexcept : _data(data)
    {
    }

    /// @brief Generate geometry for the background, defined by the inner area of the border metrics.
    void DrawBackground(const BorderMetrics &metrics, Gfx::ColourbPremultiplied colour)
    {
      const size_t initialVertexCount = _data.TotalVertices<Vertex>();

      for (int corner = 0; corner < 4; corner++)
      {
        DrawBackgroundCorner(BoxCorner(corner), metrics.InnerPositions[corner],
                             metrics.CircleCenterPositions[corner], metrics.OuterRadii[corner],
                             metrics.InnerRadii[corner], colour);
      }

      FillBackground(initialVertexCount);
    }

    /// @brief Generate geometry for the border, defined by the intersection of the outer and inner areas of
    /// the border metrics.
    void DrawBorder(const BorderMetrics &metrics, EdgeSizes edgeSizes,
                    const Array<Gfx::ColourbPremultiplied, 4> borderColours)
    {
    }

  private:
#pragma region Background

    void DrawBackgroundCorner(BoxCorner corner, Maths::Vec2 innerPosition, Maths::Vec2 pos_circle_center,
                              float R, Maths::Vec2 r, Gfx::ColourbPremultiplied color)
    {
      if (R == 0 || r.x <= 0 || r.y <= 0)
      {
        DrawPoint(innerPosition, color);
      }
      else if (r.x > 0 && r.y > 0)
      {
        const float a0 = float((int)corner + 2) * 0.5f * Maths::Pi<float>();
        const float a1 = float((int)corner + 3) * 0.5f * Maths::Pi<float>();
        DrawArc(pos_circle_center, r, a0, a1, color, color, GetNumPoints(R));
      }
    }

    void DrawPoint(Maths::Vec2 position, Gfx::ColourbPremultiplied colour)
    {
      const size_t vertexOffset = _data.TotalVertices<Vertex>();
      _data.Vertices.resize((vertexOffset + 1u) * sizeof(Vertex));

      Vertex *vertices = reinterpret_cast<Vertex *>(_data.Vertices.data());
      vertices[vertexOffset].Position = position;
      vertices[vertexOffset].Colour = colour;
    }

    void DrawArc(Maths::Vec2 centerPosition, Maths::Vec2 r, float a0, float a1,
                 Gfx::ColourbPremultiplied color0, Gfx::ColourbPremultiplied color1, int numberOfPoints)
    {
      assert(numberOfPoints >= 2 && r.x > 0 && r.y > 0);
      const size_t vertexOffset = _data.TotalVertices<Vertex>();
      _data.Vertices.resize((vertexOffset + numberOfPoints) * sizeof(Vertex));

      Vertex *vertices = reinterpret_cast<Vertex *>(_data.Vertices.data());
      for (int i = 0; i < numberOfPoints; i++)
      {
        const float t = float(i) / float(numberOfPoints - 1);

        const float a = Maths::Lerp(t, a0, a1);

        const Maths::Vec2 unitVector(Maths::Cos(a), Maths::Sin(a));

        vertices[vertexOffset + i].Position = unitVector * r + centerPosition;
        vertices[vertexOffset + i].Colour = RoundedLerp(t, color0, color1);
      }
    }

    void FillBackground(size_t indexStart)
    {

      const size_t addedVertexCount = _data.TotalVertices<Vertex>() - indexStart;
      if (addedVertexCount < 3)
      {
        return; // can't form triangles
      }

      const size_t newTriangleCount = addedVertexCount - 2;

      // Current number of indices (as element count)
      const size_t indexCount = _data.TotalIndices<Index>();

      // Resize index buffer *in bytes*
      const size_t newIndexCount = indexCount + (3 * newTriangleCount);
      const size_t newIndexBytes = newIndexCount * sizeof(Index);
      _data.Indices.resize(newIndexBytes);

      // Cast raw bytes to uint32*
      Index *indices = reinterpret_cast<Index *>(_data.Indices.data());

      // Fill triangle fan indexing
      for (size_t i = 0; i < newTriangleCount; i++)
      {
        size_t idx = indexCount + 3 * i;
        indices[idx + 0] = static_cast<Index>(indexStart);
        indices[idx + 1] = static_cast<Index>(indexStart + i + 2);
        indices[idx + 2] = static_cast<Index>(indexStart + i + 1);
      }
    }

#pragma endregion

#pragma region Border

#pragma endregion

    int GetNumPoints(float R) const
    {
      return Maths::Clamp(3 + RoundToInteger(R / 6.f), 2, 100);
    }
  };
}
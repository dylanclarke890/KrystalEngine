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
      assert(!borderColours.empty());

      const size_t initialVertexCount = _data.TotalVertices<Vertex>();

      const bool shouldDrawEdge[4] = {
        edgeSizes[+BoxEdge::Top] > 0 && borderColours[+BoxEdge::Top].alpha > 0,
        edgeSizes[+BoxEdge::Right] > 0 && borderColours[+BoxEdge::Right].alpha > 0,
        edgeSizes[+BoxEdge::Bottom] > 0 && borderColours[+BoxEdge::Bottom].alpha > 0,
        edgeSizes[+BoxEdge::Left] > 0 && borderColours[+BoxEdge::Left].alpha > 0,
      };

      const bool shouldDrawCorner[4] = {
        shouldDrawEdge[+BoxEdge::Top] || shouldDrawEdge[+BoxEdge::Left],
        shouldDrawEdge[+BoxEdge::Top] || shouldDrawEdge[+BoxEdge::Right],
        shouldDrawEdge[+BoxEdge::Bottom] || shouldDrawEdge[+BoxEdge::Right],
        shouldDrawEdge[+BoxEdge::Bottom] || shouldDrawEdge[+BoxEdge::Left],
      };

      for (int corner = 0; corner < 4; corner++)
      {
        const BoxEdge edge0 = BoxEdge((corner + 3) % 4);
        const BoxEdge edge1 = BoxEdge(corner);

        if (shouldDrawCorner[corner])
        {
          DrawBorderCorner(BoxCorner(corner), metrics.OuterPositions[corner], metrics.InnerPositions[corner],
                           metrics.CircleCenterPositions[corner], metrics.OuterRadii[corner],
                           metrics.InnerRadii[corner], borderColours[+edge0], borderColours[+edge1]);
        }

        if (shouldDrawEdge[+edge1])
        {
          assert(shouldDrawCorner[corner] && shouldDrawCorner[(corner + 1) % 4]
                 && "Border edges can only be drawn if both of its connected corners are drawn.");

          FillEdge(edge1 == BoxEdge::Left ? (int)initialVertexCount : (int)_data.TotalVertices<Vertex>());
        }
      }
    }

  private:
#pragma region Background

    void DrawBackgroundCorner(BoxCorner corner, Maths::Vec2 innerPosition, Maths::Vec2 circleCenterPosition,
                              float R, Maths::Vec2 r, Gfx::ColourbPremultiplied colour)
    {
      if (R == 0 || r.x <= 0 || r.y <= 0)
      {
        DrawPoint(innerPosition, colour);
      }
      else if (r.x > 0 && r.y > 0)
      {
        const float a0 = float((int)corner + 2) * 0.5f * Maths::Pi<float>();
        const float a1 = float((int)corner + 3) * 0.5f * Maths::Pi<float>();
        DrawArc(circleCenterPosition, r, a0, a1, colour, colour, GetNumPoints(R));
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
                 Gfx::ColourbPremultiplied colour0, Gfx::ColourbPremultiplied colour1, int numberOfPoints)
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
        vertices[vertexOffset + i].Colour = RoundedLerp(t, colour0, colour1);
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

      const size_t indexCount = _data.TotalIndices<Index>();
      const size_t newIndexCount = indexCount + (3 * newTriangleCount);
      const size_t newIndexBytes = newIndexCount * sizeof(Index);
      _data.Indices.resize(newIndexBytes);
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

    void DrawBorderCorner(BoxCorner corner, Maths::Vec2 outerPosition, Maths::Vec2 innerPosition,
                          Maths::Vec2 circleCenterPosition, float R, Maths::Vec2 r,
                          Gfx::ColourbPremultiplied colour0, Gfx::ColourbPremultiplied colour1)
    {
      const float a0 = float((int)corner + 2) * 0.5f * Maths::Pi<float>();
      const float a1 = float((int)corner + 3) * 0.5f * Maths::Pi<float>();

      if (R == 0)
      {
        DrawPointPoint(outerPosition, innerPosition, colour0, colour1);
      }
      else if (r.x > 0 && r.y > 0)
      {
        DrawArcArc(circleCenterPosition, R, r, a0, a1, colour0, colour1, GetNumPoints(R));
      }
      else
      {
        DrawArcPoint(circleCenterPosition, innerPosition, R, a0, a1, colour0, colour1, GetNumPoints(R));
      }
    }

    void DrawPointPoint(Maths::Vec2 outerPosition, Maths::Vec2 innerPosition,
                        Gfx::ColourbPremultiplied colour0, Gfx::ColourbPremultiplied colour1)
    {
      const bool isColourDifferent = (colour0 != colour1);

      _data.Vertices.reserve((_data.TotalVertices<Vertex>() + (isColourDifferent ? 4 : 2)) * sizeof(Vertex));

      DrawPoint(innerPosition, colour0);
      DrawPoint(outerPosition, colour0);

      if (isColourDifferent)
      {
        DrawPoint(innerPosition, colour1);
        DrawPoint(outerPosition, colour1);
      }
    }

    void DrawArcArc(Maths::Vec2 pos_center, float R, Maths::Vec2 r, float a0, float a1,
                    Gfx::ColourbPremultiplied color0, Gfx::ColourbPremultiplied color1, int numberOfPoints)
    {
      assert(numberOfPoints >= 2 && R > 0 && r.x > 0 && r.y > 0);

      const size_t numberOfTriangles = 2u * ((size_t)numberOfPoints - 1u);

      const size_t vertexOffset = _data.TotalVertices<Vertex>();
      const size_t indexOffset = _data.TotalIndices<Index>();

      _data.Vertices.resize((vertexOffset + 2u * (size_t)numberOfPoints) * sizeof(Vertex));
      Vertex *vertices = reinterpret_cast<Vertex *>(_data.Vertices.data());
      _data.Indices.resize((indexOffset + 3u * numberOfTriangles) * sizeof(Index));
      Index *indices = reinterpret_cast<Index *>(_data.Indices.data());

      for (size_t i = 0; i < numberOfPoints; i++)
      {
        const float t = float(i) / float(numberOfPoints - 1);

        const float a = Maths::Lerp(t, a0, a1);
        const Gfx::ColourbPremultiplied color = RoundedLerp(t, color0, color1);
        const Maths::Vec2 unit_vector(Maths::Cos(a), Maths::Sin(a));

        vertices[vertexOffset + 2u * i].Position = unit_vector * r + pos_center;
        vertices[vertexOffset + 2u * i].Colour = color;
        vertices[vertexOffset + 2u * i + 1].Position = unit_vector * R + pos_center;
        vertices[vertexOffset + 2u * i + 1].Colour = color;
      }

      for (size_t i = 0u; i < numberOfTriangles; i += 2u)
      {
        indices[indexOffset + 3u * i + 0u] = Index(vertexOffset + i + 0u);
        indices[indexOffset + 3u * i + 1u] = Index(vertexOffset + i + 2u);
        indices[indexOffset + 3u * i + 2u] = Index(vertexOffset + i + 1u);

        indices[indexOffset + 3u * i + 3u] = Index(vertexOffset + i + 1u);
        indices[indexOffset + 3u * i + 4u] = Index(vertexOffset + i + 2u);
        indices[indexOffset + 3u * i + 5u] = Index(vertexOffset + i + 3u);
      }
    }

    void DrawArcPoint(Maths::Vec2 centerPosition, Maths::Vec2 innerPosition, float R, float a0, float a1,
                      Gfx::ColourbPremultiplied colour0, Gfx::ColourbPremultiplied colour1,
                      int numberOfPoints)
    {
      assert(R > 0 && numberOfPoints >= 2);

      const size_t vertexOffset = _data.TotalVertices<Vertex>();
      _data.Vertices.reserve((vertexOffset + numberOfPoints + 2) * sizeof(Vertex));
      Vertex *vertices = reinterpret_cast<Vertex *>(_data.Vertices.data());

      // Generate the vertices. We could also split the arc mid-way to create a sharp color transition.
      DrawPoint(innerPosition, colour0);
      DrawArc(centerPosition, Maths::Vec2(R), a0, a1, colour0, colour1, numberOfPoints);
      DrawPoint(innerPosition, colour1);

      assert(_data.TotalVertices<Vertex>() - vertexOffset == numberOfPoints + 2);

      // Swap the last two vertices such that the outer edge vertex is last, see the comment for the border
      // drawing functions. Their colors should already be the same.
      const int lastVertex = (int)_data.TotalVertices<Vertex>() - 1u;
      std::swap(vertices[lastVertex - 1u].Position, vertices[lastVertex].Position);

      // Generate the indices
      const size_t numberOfTriangles = ((size_t)numberOfPoints - 1u);

      const size_t i_vertex_inner0 = vertexOffset;
      const size_t i_vertex_inner1 = (size_t)lastVertex - 1u;

      const size_t indexOffset = _data.TotalIndices<Index>();
      _data.Indices.resize((indexOffset + 3u * numberOfTriangles) * sizeof(Index));
      Index *indices = reinterpret_cast<Index *>(_data.Indices.data());

      for (size_t i = 0; i < numberOfTriangles; i++)
      {
        indices[indexOffset + 3u * i + 0u] =
          Index(i > numberOfTriangles / 2u ? i_vertex_inner1 : i_vertex_inner0);
        indices[indexOffset + 3u * i + 1u] = Index(vertexOffset + i + 2u);
        indices[indexOffset + 3u * i + 2u] = Index(vertexOffset + i + 1u);
      }

      // Since we swapped the last two vertices we also need to change the last triangle.
      indices[indexOffset + 3u * (numberOfTriangles - 1u) + 1u] = lastVertex;
    }

    void FillEdge(int nextCornerIndex)
    {
      const size_t indexOffset = _data.TotalIndices<Index>();
      const size_t numberOfVertices = _data.TotalVertices<Vertex>();
      assert(numberOfVertices >= 2u);

      _data.Indices.resize((indexOffset + 6u) * sizeof(Index));
      Index *indices = reinterpret_cast<Index *>(_data.Indices.data());

      indices[indexOffset + 0u] = Index(numberOfVertices - 2u);
      indices[indexOffset + 1u] = Index(nextCornerIndex);
      indices[indexOffset + 2u] = Index(numberOfVertices - 1u);

      indices[indexOffset + 3u] = Index(numberOfVertices - 1u);
      indices[indexOffset + 4u] = Index(nextCornerIndex);
      indices[indexOffset + 5u] = Index(nextCornerIndex + 1u);
    }

#pragma endregion

    int GetNumPoints(float R) const
    {
      return Maths::Clamp(3 + RoundToInteger(R / 6.f), 2, 100);
    }
  };
}
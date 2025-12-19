#pragma once

#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Gfx/Resources/Mesh.hpp"
#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Maths.hpp"
#include "Krystal.UI/Geometry/BorderMetrics.hpp"
#include "Krystal.UI/Geometry/Common.hpp"

namespace Krys::UI
{
  class GeometryBackgroundBorder
  {
    NO_COPY_MOVE(GeometryBackgroundBorder)

  private:
    GeometryMeshWriter &_writer;

  public:
    GeometryBackgroundBorder(GeometryMeshWriter &writer) noexcept : _writer(writer)
    {
    }

    /// @brief Generate geometry for the background, defined by the inner area of the border metrics.
    void DrawBackground(const BorderMetrics &metrics, Gfx::ColourbPremultiplied colour)
    {
      const size_t initialVertexCount = _writer.TotalVertices();

      for (size_t corner = 0u; corner < 4u; corner++)
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

      const size_t initialVertexCount = _writer.TotalVertices();

      const bool shouldDrawEdge[4] = {
        edgeSizes[+BoxEdge::Top] > 0u && borderColours[+BoxEdge::Top].alpha > 0u,
        edgeSizes[+BoxEdge::Right] > 0u && borderColours[+BoxEdge::Right].alpha > 0u,
        edgeSizes[+BoxEdge::Bottom] > 0u && borderColours[+BoxEdge::Bottom].alpha > 0u,
        edgeSizes[+BoxEdge::Left] > 0u && borderColours[+BoxEdge::Left].alpha > 0u,
      };

      const bool shouldDrawCorner[4] = {
        shouldDrawEdge[+BoxEdge::Top] || shouldDrawEdge[+BoxEdge::Left],
        shouldDrawEdge[+BoxEdge::Top] || shouldDrawEdge[+BoxEdge::Right],
        shouldDrawEdge[+BoxEdge::Bottom] || shouldDrawEdge[+BoxEdge::Right],
        shouldDrawEdge[+BoxEdge::Bottom] || shouldDrawEdge[+BoxEdge::Left],
      };

      for (size_t corner = 0u; corner < 4u; corner++)
      {
        const BoxEdge edge0 = BoxEdge((corner + 3u) % 4u);
        const BoxEdge edge1 = BoxEdge(corner);

        if (shouldDrawCorner[corner])
        {
          DrawBorderCorner(BoxCorner(corner), metrics.OuterPositions[corner], metrics.InnerPositions[corner],
                           metrics.CircleCenterPositions[corner], metrics.OuterRadii[corner],
                           metrics.InnerRadii[corner], borderColours[+edge0], borderColours[+edge1]);
        }

        if (shouldDrawEdge[+edge1])
        {
          assert(shouldDrawCorner[corner] && shouldDrawCorner[(corner + 1u) % 4u]
                 && "Border edges can only be drawn if both of its connected corners are drawn.");

          FillEdge(edge1 == BoxEdge::Left ? initialVertexCount : _writer.TotalVertices());
        }
      }
    }

  private:
#pragma region Background

    void DrawBackgroundCorner(BoxCorner corner, Maths::Vec2 innerPosition, Maths::Vec2 circleCenterPosition,
                              float R, Maths::Vec2 r, Gfx::ColourbPremultiplied colour)
    {
      if (R == 0u || r.x <= 0u || r.y <= 0u)
      {
        DrawPoint(innerPosition, colour);
      }
      else if (r.x > 0u && r.y > 0u)
      {
        const float a0 = float(+corner + 2u) * 0.5f * Maths::Pi<float>();
        const float a1 = float(+corner + 3u) * 0.5f * Maths::Pi<float>();
        DrawArc(circleCenterPosition, r, a0, a1, colour, colour, GetNumPoints(R));
      }
    }

    void DrawPoint(Maths::Vec2 position, Gfx::ColourbPremultiplied colour)
    {
      const size_t vertexOffset = _writer.TotalVertices();
      _writer.ResizeVertices(vertexOffset + 1u);

      auto *vertices = _writer.Vertices();
      vertices[vertexOffset].Position = position;
      vertices[vertexOffset].Colour = colour;
    }

    void DrawArc(Maths::Vec2 centerPosition, Maths::Vec2 r, float a0, float a1,
                 Gfx::ColourbPremultiplied colour0, Gfx::ColourbPremultiplied colour1, size_t numberOfPoints)
    {
      assert(numberOfPoints >= 2 && r.x > 0 && r.y > 0);

      const size_t vertexOffset = _writer.TotalVertices();
      _writer.ResizeVertices(vertexOffset + numberOfPoints);

      auto *vertices = _writer.Vertices();
      for (size_t i = 0u; i < numberOfPoints; i++)
      {
        const float t = float(i) / float(numberOfPoints - 1u);
        const float a = Maths::Lerp(a0, a1, t);
        const Maths::Vec2 unitVector(Maths::Cos(a), Maths::Sin(a));

        vertices[vertexOffset + i].Position = unitVector * r + centerPosition;
        vertices[vertexOffset + i].Colour = RoundedLerp(colour0, colour1, t);
      }
    }

    void FillBackground(size_t indexStart)
    {
      const size_t addedVertexCount = _writer.TotalVertices() - indexStart;
      if (addedVertexCount < 3)
      {
        return; // can't form triangles
      }
      const size_t newTriangleCount = addedVertexCount - 2u;

      const size_t indexOffset = _writer.TotalIndices();
      _writer.ResizeIndices(indexOffset + (3 * newTriangleCount));

      // Fill triangle fan indexing
      Index *indices = _writer.Indices();
      for (size_t i = 0; i < newTriangleCount; i++)
      {
        size_t index = indexOffset + 3u * i;
        indices[index + 0u] = Index(indexStart);
        indices[index + 1u] = Index(indexStart + i + 2u);
        indices[index + 2u] = Index(indexStart + i + 1u);
      }
    }

#pragma endregion

#pragma region Border

    void DrawBorderCorner(BoxCorner corner, Maths::Vec2 outerPosition, Maths::Vec2 innerPosition,
                          Maths::Vec2 circleCenterPosition, float R, Maths::Vec2 r,
                          Gfx::ColourbPremultiplied colour0, Gfx::ColourbPremultiplied colour1)
    {
      const float a0 = float(+corner + 2u) * 0.5f * Maths::Pi<float>();
      const float a1 = float(+corner + 3u) * 0.5f * Maths::Pi<float>();

      if (R == 0u)
      {
        DrawPointPoint(outerPosition, innerPosition, colour0, colour1);
      }
      else if (r.x > 0u && r.y > 0u)
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

      _writer.ReserveVertices(_writer.TotalVertices() + (isColourDifferent ? 4u : 2u));

      DrawPoint(innerPosition, colour0);
      DrawPoint(outerPosition, colour0);

      if (isColourDifferent)
      {
        DrawPoint(innerPosition, colour1);
        DrawPoint(outerPosition, colour1);
      }
    }

    void DrawArcArc(Maths::Vec2 centerPosition, float R, Maths::Vec2 r, float a0, float a1,
                    Gfx::ColourbPremultiplied colour0, Gfx::ColourbPremultiplied colour1,
                    size_t numberOfPoints)
    {
      assert(numberOfPoints >= 2u && R > 0u && r.x > 0u && r.y > 0u);

      const size_t vertexOffset = _writer.TotalVertices();
      _writer.ResizeVertices(vertexOffset + 2u * (size_t)numberOfPoints);
      auto *vertices = _writer.Vertices();

      const size_t numberOfTriangles = 2u * ((size_t)numberOfPoints - 1u);
      const size_t indexOffset = _writer.TotalIndices();
      _writer.ResizeIndices(indexOffset + 3u * numberOfTriangles);
      auto *indices = _writer.Indices();

      for (size_t i = 0u; i < numberOfPoints; i++)
      {
        const float t = float(i) / float(numberOfPoints - 1);
        const float a = Maths::Lerp(a0, a1, t);

        const Gfx::ColourbPremultiplied colour = RoundedLerp(colour0, colour1, t);
        const Maths::Vec2 unitVector(Maths::Cos(a), Maths::Sin(a));

        vertices[vertexOffset + 2u * i].Position = unitVector * r + centerPosition;
        vertices[vertexOffset + 2u * i].Colour = colour;
        vertices[vertexOffset + 2u * i + 1].Position = unitVector * R + centerPosition;
        vertices[vertexOffset + 2u * i + 1].Colour = colour;
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
                      size_t numberOfPoints)
    {
      assert(R > 0u && numberOfPoints >= 2u);

      const size_t vertexOffset = _writer.TotalVertices();
      _writer.ReserveVertices(vertexOffset + numberOfPoints + 2u);

      // Generate the vertices. We could also split the arc mid-way to create a sharp color transition.
      DrawPoint(innerPosition, colour0);
      DrawArc(centerPosition, Maths::Vec2(R), a0, a1, colour0, colour1, numberOfPoints);
      DrawPoint(innerPosition, colour1);

      assert(_writer.TotalVertices() - vertexOffset == numberOfPoints + 2);

      auto *vertices = _writer.Vertices();
      // Swap the last two vertices such that the outer edge vertex is last, see the comment for the border
      // drawing functions. Their colors should already be the same.
      const size_t lastVertex = _writer.TotalVertices() - 1u;
      std::swap(vertices[lastVertex - 1u].Position, vertices[lastVertex].Position);

      // Generate the indices
      const size_t numberOfTriangles = ((size_t)numberOfPoints - 1u);

      const size_t i_vertex_inner0 = vertexOffset;
      const size_t i_vertex_inner1 = lastVertex - 1u;

      const size_t indexOffset = _writer.TotalIndices();
      _writer.ResizeIndices(indexOffset + 3u * numberOfTriangles);
      auto *indices = _writer.Indices();

      for (size_t i = 0u; i < numberOfTriangles; i++)
      {
        indices[indexOffset + 3u * i + 0u] =
          Index(i > numberOfTriangles / 2u ? i_vertex_inner1 : i_vertex_inner0);
        indices[indexOffset + 3u * i + 1u] = Index(vertexOffset + i + 2u);
        indices[indexOffset + 3u * i + 2u] = Index(vertexOffset + i + 1u);
      }

      // Since we swapped the last two vertices we also need to change the last triangle.
      indices[indexOffset + 3u * (numberOfTriangles - 1u) + 1u] = Index(lastVertex);
    }

    void FillEdge(size_t nextCornerIndex)
    {
      const size_t indexOffset = _writer.TotalIndices();
      const size_t numberOfVertices = _writer.TotalVertices();
      assert(numberOfVertices >= 2u);

      _writer.ResizeIndices(indexOffset + 6u);
      auto *indices = _writer.Indices();

      indices[indexOffset + 0u] = Index(numberOfVertices - 2u);
      indices[indexOffset + 1u] = Index(nextCornerIndex);
      indices[indexOffset + 2u] = Index(numberOfVertices - 1u);

      indices[indexOffset + 3u] = Index(numberOfVertices - 1u);
      indices[indexOffset + 4u] = Index(nextCornerIndex);
      indices[indexOffset + 5u] = Index(nextCornerIndex + 1u);
    }

#pragma endregion

    size_t GetNumPoints(float R) const
    {
      return Maths::Clamp(3uz + RoundTo<size_t>(R / 6.f), 2uz, 100uz);
    }
  };
}
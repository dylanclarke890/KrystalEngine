#pragma once

#include "Krystal.Gfx/Utils/MeshDataUtils.hpp"
#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Clamp.hpp"
#include "Krystal.Maths/Round.hpp"
#include "Krystal.Maths/Vector.hpp"
#include "Krystal.UI/Geometry/BorderMetrics.hpp"
#include "Krystal.UI/Geometry/GeometryBackgroundBorder.hpp"
#include "Krystal.UI/Geometry/RenderBox.hpp"

namespace Krys::UI
{
  class GeometryUtils
  {
    STATIC_CLASS(GeometryUtils)

  public:
    static void GenerateBorderBackground(Gfx::MeshData &data, const RenderBox &renderBox,
                                         Gfx::ColourbPremultiplied backgroundColour,
                                         const Array<Gfx::ColourbPremultiplied, 4> &borderColours)
    {
      using namespace Gfx;
      using namespace Maths;

      auto &borderWidths = renderBox.GetBorderWidths();

      int numberOfBorders = 0;
      for (int i = 0; i < 4; i++)
      {
        if (borderColours[i].alpha > 0 && borderWidths[i] > 0)
        {
          numberOfBorders++;
        }
      }

      const Vec2 &fillSize = renderBox.GetFillSize();
      const bool hasBackground = (backgroundColour.alpha > 0 && fillSize.x > 0 && fillSize.y > 0);
      const bool hasBorder = numberOfBorders > 0;

      if (!hasBackground && !hasBorder)
      {
        return;
      }

      using Vertex = Gfx::Vertex::Position2D_ColourbPremultiplied_UV;
      using Index = uint32;

      // Reserve geometry. A conservative estimate, does not take border-radii into account and assumes
      // same-colored borders.
      const size_t estimatedVertexCount = 4 * size_t(hasBackground) + 2 * (size_t)numberOfBorders;
      const size_t estimatedTriangleCount = 2 * size_t(hasBackground) + 2 * (size_t)numberOfBorders;
      data.Vertices.reserve((data.TotalVertices<Vertex>() + estimatedVertexCount) * sizeof(Vertex));
      data.Indices.reserve((data.TotalIndices<Index>() + 3 * estimatedTriangleCount) * sizeof(Index));

      // Generate the geometry.
      GeometryBackgroundBorder geometry(data);
      const BorderMetrics metrics = ComputeBorderMetrics(renderBox.GetBorderOffset(), borderWidths, fillSize,
                                                         renderBox.GetBorderRadius());

      if (hasBackground)
      {
        geometry.DrawBackground(metrics, backgroundColour);
      }

      if (hasBorder)
      {
        geometry.DrawBorder(metrics, borderWidths, borderColours);
      }

#ifdef _DEBUG
      const size_t numberOfVertices = data.TotalVertices<Vertex>();
      const size_t indexCount = data.TotalIndices<Index>();
      Index *indices = reinterpret_cast<Index *>(data.Indices.data());

      for (size_t i = 0; i < indexCount; i++)
      {
        assert(indices[i] < numberOfVertices);
      }
#endif
    }

    static BorderMetrics ComputeBorderMetrics(Maths::Vec2 outerPosition, EdgeSizes edgeSizes,
                                              Maths::Vec2 innerSize, CornerSizes initialOuterRadii)
    {
      using namespace Maths;

      BorderMetrics metrics = {};

      // -- Find the corner positions --

      const Vec2 innerPosition = outerPosition + Vec2(edgeSizes[+BoxEdge::Left], edgeSizes[+BoxEdge::Top]);
      const Vec2 outerSize = innerSize
                             + Vec2(edgeSizes[+BoxEdge::Left] + edgeSizes[+BoxEdge::Right],
                                    edgeSizes[+BoxEdge::Top] + edgeSizes[+BoxEdge::Bottom]);

      metrics.OuterPositions = {
        outerPosition,
        outerPosition + Vec2(outerSize.x, 0.f),
        outerPosition + outerSize,
        outerPosition + Vec2(0.f, outerSize.y),
      };

      metrics.InnerPositions = {
        innerPosition,
        innerPosition + Vec2(innerSize.x, 0),
        innerPosition + innerSize,
        innerPosition + Vec2(0, innerSize.y),
      };

      // -- For curved borders, find the positions to draw ellipses around, and the scaled outer and inner
      // radii --

      const float sumOfRadii =
        (initialOuterRadii[+BoxCorner::TopLeft] + initialOuterRadii[+BoxCorner::TopRight]
         + initialOuterRadii[+BoxCorner::BottomRight] + initialOuterRadii[+BoxCorner::BottomLeft]);
      const bool hasRadius = (sumOfRadii > 1.f);

      if (hasRadius)
      {
        auto &outerRadii = metrics.OuterRadii;
        outerRadii = initialOuterRadii;

        // Scale the radii such that we have no overlapping curves.
        float scale = FLT_MAX;
        // Top
        scale =
          Min(scale, innerSize.x / (outerRadii[+BoxCorner::TopLeft] + outerRadii[+BoxCorner::TopRight]));

        // Right
        scale =
          Min(scale, innerSize.y / (outerRadii[+BoxCorner::TopRight] + outerRadii[+BoxCorner::BottomRight]));

        // Bottom
        scale = Min(scale,
                    innerSize.x / (outerRadii[+BoxCorner::BottomRight] + outerRadii[+BoxCorner::BottomLeft]));

        // Left
        scale =
          Min(scale, innerSize.y / (outerRadii[+BoxCorner::BottomLeft] + outerRadii[+BoxCorner::TopLeft]));

        scale = Min(1.0f, scale);

        for (float &radius : outerRadii)
        {
          radius = Round(radius * scale);
        }

        // Place the circle/ellipse centers
        metrics.CircleCenterPositions = {
          metrics.OuterPositions[+BoxCorner::TopLeft] + Vec2(1, 1) * outerRadii[+BoxCorner::TopLeft],
          metrics.OuterPositions[+BoxCorner::TopRight] + Vec2(-1, 1) * outerRadii[+BoxCorner::TopRight],
          metrics.OuterPositions[+BoxCorner::BottomRight]
            + Vec2(-1, -1) * outerRadii[+BoxCorner::BottomRight],
          metrics.OuterPositions[+BoxCorner::BottomLeft] + Vec2(1, -1) * outerRadii[+BoxCorner::BottomLeft],
        };

        metrics.InnerRadii = {
          Vec2(outerRadii[+BoxCorner::TopLeft]) - Vec2(edgeSizes[+BoxEdge::Left], edgeSizes[+BoxEdge::Top]),
          Vec2(outerRadii[+BoxCorner::TopRight]) - Vec2(edgeSizes[+BoxEdge::Right], edgeSizes[+BoxEdge::Top]),
          Vec2(outerRadii[+BoxCorner::BottomRight])
            - Vec2(edgeSizes[+BoxEdge::Right], edgeSizes[+BoxEdge::Bottom]),
          Vec2(outerRadii[+BoxCorner::BottomLeft])
            - Vec2(edgeSizes[+BoxEdge::Left], edgeSizes[+BoxEdge::Bottom]),
        };
      }

      return metrics;
    }
  };
}
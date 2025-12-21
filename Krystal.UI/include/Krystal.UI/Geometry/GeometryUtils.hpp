#pragma once

#include "Krystal.Gfx/Utils/MeshDataUtils.hpp"
#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Clamp.hpp"
#include "Krystal.Maths/Round.hpp"
#include "Krystal.Maths/Vector.hpp"
#include "Krystal.UI/Geometry/BorderMetrics.hpp"
#include "Krystal.UI/Geometry/Common.hpp"
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

      size_t numberOfBorders = 0u;
      for (size_t i = 0u; i < 4u; i++)
      {
        if (borderColours[i].alpha > 0u && borderWidths[i] > 0u)
        {
          numberOfBorders++;
        }
      }

      const Vec2 &fillSize = renderBox.GetFillSize();
      const bool hasBackground = (backgroundColour.alpha > 0u && fillSize.x > 0u && fillSize.y > 0u);
      const bool hasBorder = numberOfBorders > 0u;

      if (!hasBackground && !hasBorder)
      {
        return;
      }

      const BorderMetrics metrics = ComputeBorderMetrics(renderBox.GetBorderOffset(), borderWidths, fillSize,
                                                         renderBox.GetBorderRadius());

      // Reserve geometry. A conservative estimate, does not take border-radii into account and assumes
      // same-colored borders.
      GeometryMeshWriter geometryWriter(data);

      const size_t estimatedVertexCount = 4u * size_t(hasBackground) + 2u * numberOfBorders;
      geometryWriter.ReserveVertices(geometryWriter.TotalVertices() + estimatedVertexCount);

      const size_t estimatedTriangleCount = 2u * size_t(hasBackground) + 2u * numberOfBorders;
      geometryWriter.ReserveIndices(geometryWriter.TotalIndices() + 3u * estimatedTriangleCount);

      GeometryBackgroundBorder geometry(geometryWriter);
      if (hasBackground)
      {
        geometry.DrawBackground(metrics, backgroundColour);
      }

      if (hasBorder)
      {
        geometry.DrawBorder(metrics, borderWidths, borderColours);
      }

#ifdef KRYS_BUILD_DEBUG
      const size_t numberOfVertices = geometryWriter.TotalVertices();
      const size_t indexCount = geometryWriter.TotalIndices();
      auto *indices = geometryWriter.Indices();
      auto *vertices = geometryWriter.Vertices();

      for (size_t i = 0u; i < indexCount; i++)
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

      // Find the corner positions

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

      // For curved borders, find the positions to draw ellipses around, and the scaled outer and inner
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
        float radiiScale[4] = {
          outerSize.x / (outerRadii[+BoxCorner::TopLeft] + outerRadii[+BoxCorner::TopRight]),
          outerSize.y / (outerRadii[+BoxCorner::TopRight] + outerRadii[+BoxCorner::BottomRight]),
          outerSize.x / (outerRadii[+BoxCorner::BottomRight] + outerRadii[+BoxCorner::BottomLeft]),
          outerSize.y / (outerRadii[+BoxCorner::BottomLeft] + outerRadii[+BoxCorner::TopLeft]),
        };

        float scale = FLT_MAX;
        for (size_t i = 0u; i < 4u; i++)
        {
          scale = Min(scale, radiiScale[i]);
        }
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
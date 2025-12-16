#pragma once

#include "Krystal.Gfx/Utils/MeshDataUtils.hpp"
#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Clamp.hpp"
#include "Krystal.Maths/Round.hpp"
#include "Krystal.Maths/Vector.hpp"
#include "Krystal.UI/Geometry/BorderMetrics.hpp"
#include "Krystal.UI/Geometry/RenderBox.hpp"

namespace Krys::UI
{
  class GeometryUtils
  {
    STATIC_CLASS(GeometryUtils)

  public:
    static void GenerateBorderBackground(Gfx::MeshData &data, const RenderBox &renderBox,
                                         Gfx::ColourbPremultiplied backgroundColour,
                                         const Array<Gfx::ColourbPremultiplied, 4> &borderColors)
    {
      using namespace Gfx;
      using namespace Maths;

      auto &borderWidths = renderBox.GetBorderWidths();

      int numberOfBorders = 0;
      for (int i = 0; i < 4; i++)
      {
        if (borderColors[i].alpha > 0 && borderWidths[i] > 0)
        {
          numberOfBorders++;
        }
      }

      const Vec2 &fill_size = renderBox.GetFillSize();
      const bool hasBackground = (backgroundColour.alpha > 0 && fill_size.x > 0 && fill_size.y > 0);
      const bool hasBorder = numberOfBorders > 0;

      if (!hasBackground && !hasBorder)
      {
        return;
      }

      MeshDataUtils::GenerateQuad(data, renderBox.GetFillOffset(), renderBox.GetFillSize(), backgroundColour);

      if (borderWidths[TopEdge] > 0.f && borderColors[TopEdge].alpha > 0.f)
      {
        Vec2 size = {renderBox.GetFillSize().x, borderWidths[TopEdge]};
        MeshDataUtils::GenerateQuad(data, {borderWidths[LeftEdge], 0.f}, size, borderColors[TopEdge]);
      }

      if (borderWidths[LeftEdge] > 0.f && borderColors[LeftEdge].alpha > 0.f)
      {
        Vec2 size = {borderWidths[LeftEdge],
                     renderBox.GetFillSize().y + borderWidths[TopEdge] + borderWidths[BottomEdge]};
        MeshDataUtils::GenerateQuad(data, {0.f, 0.f}, size, borderColors[LeftEdge]);
      }

      if (borderWidths[BottomEdge] > 0.f && borderColors[BottomEdge].alpha > 0.f)
      {
        Vec2 size = {renderBox.GetFillSize().x, borderWidths[BottomEdge]};
        MeshDataUtils::GenerateQuad(
          data, {borderWidths[LeftEdge], renderBox.GetFillSize().y + borderWidths[TopEdge]}, size,
          borderColors[BottomEdge]);
      }

      if (borderWidths[RightEdge] > 0.f && borderColors[RightEdge].alpha > 0.f)
      {
        Vec2 size = {borderWidths[RightEdge],
                     renderBox.GetFillSize().y + borderWidths[TopEdge] + borderWidths[BottomEdge]};
        MeshDataUtils::GenerateQuad(data, {renderBox.GetFillSize().x + borderWidths[LeftEdge], 0.f}, size,
                                    borderColors[RightEdge]);
      }
    }

    static BorderMetrics ComputeBorderMetrics(Maths::Vec2 outerPosition, EdgeSizes edgeSizes,
                                              Maths::Vec2 innerSize, CornerSizes initialOuterRadii)
    {
      using namespace Maths;

      BorderMetrics metrics = {};

      // -- Find the corner positions --

      const Vec2 innerPosition = outerPosition + Vec2(edgeSizes[LeftEdge], edgeSizes[TopEdge]);
      const Vec2 outerSize =
        innerSize
        + Vec2(edgeSizes[LeftEdge] + edgeSizes[RightEdge], edgeSizes[TopEdge] + edgeSizes[BottomEdge]);

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

      const float sumOfRadii = (initialOuterRadii[TopLeftCorner] + initialOuterRadii[TopRightCorner]
                                + initialOuterRadii[BottomRightCorner] + initialOuterRadii[BottomLeftCorner]);
      const bool hasRadius = (sumOfRadii > 1.f);

      if (hasRadius)
      {
        auto &outerRadii = metrics.OuterRadii;
        outerRadii = initialOuterRadii;

        // Scale the radii such that we have no overlapping curves.
        float scale = FLT_MAX;
        // Top
        scale = Min(scale, innerSize.x / (outerRadii[TopLeftCorner] + outerRadii[TopRightCorner]));

        // Right
        scale = Min(scale, innerSize.y / (outerRadii[TopRightCorner] + outerRadii[BottomRightCorner]));

        // Bottom
        scale = Min(scale, innerSize.x / (outerRadii[BottomRightCorner] + outerRadii[BottomLeftCorner]));

        // Left
        scale = Min(scale, innerSize.y / (outerRadii[BottomLeftCorner] + outerRadii[TopLeftCorner]));

        scale = Min(1.0f, scale);

        for (float &radius : outerRadii)
        {
          radius = Round(radius * scale);
        }

        // Place the circle/ellipse centers
        metrics.CircleCenterPositions = {
          metrics.OuterPositions[TopLeftCorner] + Vec2(1, 1) * outerRadii[TopLeftCorner],
          metrics.OuterPositions[TopRightCorner] + Vec2(-1, 1) * outerRadii[TopRightCorner],
          metrics.OuterPositions[BottomRightCorner] + Vec2(-1, -1) * outerRadii[BottomRightCorner],
          metrics.OuterPositions[BottomLeftCorner] + Vec2(1, -1) * outerRadii[BottomLeftCorner],
        };

        metrics.InnerRadii = {
          Vec2(outerRadii[TopLeftCorner]) - Vec2(edgeSizes[LeftEdge], edgeSizes[TopEdge]),
          Vec2(outerRadii[TopRightCorner]) - Vec2(edgeSizes[RightEdge], edgeSizes[TopEdge]),
          Vec2(outerRadii[BottomRightCorner]) - Vec2(edgeSizes[RightEdge], edgeSizes[BottomEdge]),
          Vec2(outerRadii[BottomLeftCorner]) - Vec2(edgeSizes[LeftEdge], edgeSizes[BottomEdge]),
        };
      }

      return metrics;
    }
  };
}
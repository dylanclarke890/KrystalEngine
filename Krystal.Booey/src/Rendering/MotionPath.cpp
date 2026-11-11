/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Rendering/MotionPath.hpp"
#include "Krystal.Booey/Graphics/Geometry/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"
#include "Krystal.Booey/Graphics/Paths/PathTraversalState.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Rendering/Painting/BorderShape.hpp"
#include "Krystal.Booey/Rendering/PathOperation.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/TransformOperationData.hpp"
#include "Krystal.Booey/Style/Values/Motion/OffsetPath.hpp"
#include "Krystal.Booey/Style/Values/Motion/OffsetPosition.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Position.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD FloatPoint MotionPathOffsetFromContainer(const RenderObject &renderer,
                                                            const RenderBlock &container,
                                                            const FloatRect &referenceRect) noexcept
    {
      auto offsetFromContainingBlock = renderer.OffsetFromContainer(container, LayoutPoint());
      return FloatPoint(FloatPoint(offsetFromContainingBlock) - referenceRect.Location());
    }

    KRYS_NODISCARD FloatRoundedRect
      ContainingBlockRectForRenderer(const RenderObject &renderer, const RenderBlock &container,
                                     const style::OffsetPath &offsetPath) noexcept
    {
      if (auto box = offsetPath.TryBox())
      {
        auto referenceRect = container.ReferenceBoxRect(box->ReferenceBox());
        auto borderShape = BorderShape::ShapeForBorderRect(container.Style(), LayoutRect(referenceRect));
        return borderShape.DeprecatedPixelSnappedRoundedRect(container.GetDocument().DeviceScaleFactor());
      }

      auto referenceBox =
        offsetPath.SwitchOn([](const css::keywords::None &) { return CSSBoxType::BoxMissing; },
                            [](const auto &path) { return path.ReferenceBox(); });
      return FloatRoundedRect {SnapRectToDevicePixelsIfNeeded(container.ReferenceBoxRect(referenceBox),
                                                              Downcast<RenderLayerModelObject>(renderer))};
    }

    KRYS_NODISCARD FloatPoint NormalPositionForOffsetPath(const style::OffsetPath &offsetPath,
                                                          const FloatRect &referenceRect) noexcept
    {
      if (offsetPath.IsRay() || offsetPath.IsBasicShape())
      {
        return referenceRect.Center();
      }
      return {};
    }
  }

  Maybe<MotionPathData> MotionPath::MotionPathDataForRenderer(const RenderElement &renderer) noexcept
  {
    if (!Is<RenderLayerModelObject>(renderer))
    {
      return null;
    }

    const auto &offsetPath = renderer.Style().GetOffsetPath();
    if (offsetPath.IsNone())
    {
      return null;
    }
    if (auto basicShape = offsetPath.TryBasicShape();
        basicShape && std::holds_alternative<style::PathFunction>(basicShape->GetShape()))
    {
      return null;
    }

    auto *container = renderer.ContainingBlock();
    if (!container)
    {
      return null;
    }

    auto startingPositionForOffsetPosition = [&](const style::OffsetPosition &offsetPosition,
                                                 const FloatRect &referenceRect) -> FloatPoint
    {
      // If offset-position is normal, the element doesn't have an offset starting position.
      if (offsetPosition.IsNormal())
      {
        return NormalPositionForOffsetPath(offsetPath, referenceRect);
      }
      // If offset-position is auto, use the top left corner of the box.
      if (offsetPosition.IsAuto())
      {
        return MotionPathOffsetFromContainer(renderer, *container, referenceRect);
      }
      return style::Evaluate<FloatPoint>(*offsetPosition.TryPosition(), referenceRect.Size(),
                                         style::ZoomNeeded {});
    };

    MotionPathData data;
    data.ContainingBlockBoundingRect = ContainingBlockRectForRenderer(renderer, *container, offsetPath);
    auto referenceRect = data.ContainingBlockBoundingRect.Rect();
    data.OffsetFromContainingBlock = MotionPathOffsetFromContainer(renderer, *container, referenceRect);

    const auto &offsetPosition = renderer.Style().GetOffsetPosition();
    if (offsetPath.IsBasicShape())
    {
      data.UsedStartingPosition = startingPositionForOffsetPosition(offsetPosition, referenceRect);
    }
    else if (auto ray = offsetPath.TryRay())
    {
      const auto &startingPosition = ray->Ray()->Position;
      data.UsedStartingPosition =
        startingPosition
          ? style::Evaluate<FloatPoint>(*startingPosition, referenceRect.Size(), style::ZoomNeeded {})
          : startingPositionForOffsetPosition(offsetPosition, referenceRect);
    }

    return data;
  }

  KRYS_NODISCARD static PathTraversalState TraversalStateAtDistance(const Path &path,
                                                                    float distanceValue) noexcept
  {
    auto pathLength = path.Length();
    float resolvedLength = 0;
    if (path.IsClosed())
    {
      if (pathLength)
      {
        resolvedLength = std::fmod(distanceValue, pathLength);
        if (resolvedLength < 0)
        {
          resolvedLength += pathLength;
        }
      }
    }
    else
    {
      resolvedLength = std::clamp(distanceValue, 0.0f, pathLength);
    }

    krys_debug_assert(resolvedLength >= 0);
    return path.TraversalStateAtLength(resolvedLength);
  }

  void MotionPath::ApplyMotionPathTransform(TransformationMatrix &matrix,
                                            const TransformOperationData &transformData,
                                            FloatPoint transformOrigin, TransformBox transformBox,
                                            const Path &offsetPath, Maybe<FloatPoint> offsetAnchor,
                                            float offsetDistance, float offsetRotate,
                                            bool offsetRotateHasAuto) noexcept
  {
    auto boundingBox = transformData.BoundingBox;
    auto anchor = transformOrigin;

    if (offsetAnchor)
    {
      anchor = *offsetAnchor + boundingBox.Location();
    }

    auto shiftToOrigin = anchor - transformOrigin;

    if (transformData.IsSVGRenderer && transformBox != TransformBox::ViewBox)
    {
      anchor += boundingBox.Location();
    }

    auto traversalState = TraversalStateAtDistance(offsetPath, offsetDistance);
    matrix.Translate(traversalState.Current().X(), traversalState.Current().Y());

    // Shift element to the anchor specified by offset-anchor.
    matrix.Translate(-anchor.X(), -anchor.Y());

    matrix.Translate(shiftToOrigin.Width(), shiftToOrigin.Height());

    // Apply rotation.
    if (offsetRotateHasAuto)
    {
      matrix.Rotate(traversalState.NormalAngle() + offsetRotate);
    }
    else
    {
      matrix.Rotate(offsetRotate);
    }

    matrix.Translate(-shiftToOrigin.Width(), -shiftToOrigin.Height());
  }

  bool MotionPath::NeedsUpdateAfterContainingBlockLayout(const style::OffsetPath &offsetPath) noexcept
  {
    return offsetPath.HoldsAlternative<style::RayPath>() || offsetPath.HoldsAlternative<style::BoxPath>()
           || offsetPath.HoldsAlternative<style::BasicShapePath>();
  }

  KRYS_NODISCARD static double LengthForRayPath(const style::Ray &ray, const MotionPathData &data) noexcept
  {
    const auto &boundingBox = data.ContainingBlockBoundingRect.Rect();
    auto distances = DistanceOfPointToSidesOfRect(boundingBox, data.UsedStartingPosition);

    return krys::SwitchOn(
      ray.Size, [&](css::keywords::ClosestSide)
      { return std::min({distances.Top(), distances.Bottom(), distances.Left(), distances.Right()}); },
      [&](css::keywords::FarthestSide)
      { return std::max({distances.Top(), distances.Bottom(), distances.Left(), distances.Right()}); },
      [&](css::keywords::FarthestCorner)
      {
        return std::hypot(std::max(distances.Left(), distances.Right()),
                          std::max(distances.Top(), distances.Bottom()));
      },
      [&](css::keywords::ClosestCorner)
      {
        return std::hypot(std::min(distances.Left(), distances.Right()),
                          std::min(distances.Top(), distances.Bottom()));
      },
      [&](css::keywords::Sides)
      {
        return LengthOfRayIntersectionWithBoundingBox(
          boundingBox, std::pair<const FloatPoint &, float> {data.UsedStartingPosition,
                                                             static_cast<float>(ray.Angle.Value)});
      });
  }

  KRYS_NODISCARD static double LengthForRayContainPath(const FloatRect &elementRect,
                                                       double computedPathLength) noexcept
  {
    return std::max(0.0, computedPathLength - (std::max(elementRect.Width(), elementRect.Height()) / 2));
  }

  KRYS_NODISCARD static FloatPoint CurrentOffsetForData(const MotionPathData &data) noexcept
  {
    return FloatPoint(data.UsedStartingPosition - data.OffsetFromContainingBlock);
  }

  Maybe<Path> MotionPath::ComputePathForRay(const RayPathOperation &rayPathOperation,
                                            const TransformOperationData &transformData) noexcept
  {
    auto motionPathData = transformData.MotionPathData;
    if (!motionPathData || motionPathData->ContainingBlockBoundingRect.Rect().IsZero())
    {
      return std::nullopt;
    }

    auto elementBoundingBox = transformData.BoundingBox;
    double length = LengthForRayPath(*rayPathOperation.Ray(), *motionPathData);
    if (rayPathOperation.Ray()->Contain)
    {
      length = LengthForRayContainPath(elementBoundingBox, length);
    }

    auto radians = deg2rad(
      static_cast<double>(ToPositiveAngle(static_cast<float>(rayPathOperation.Ray()->Angle.Value))) - 90.0);
    auto point = FloatSize(static_cast<float>(std::cos(radians) * length),
                           static_cast<float>(std::sin(radians) * length));

    Path path;
    path.MoveTo(CurrentOffsetForData(*motionPathData));
    path.AddLineTo(CurrentOffsetForData(*motionPathData) + point);
    return path;
  }

  KRYS_NODISCARD static FloatRoundedRect OffsetRectForData(const MotionPathData &data) noexcept
  {
    auto rect = data.ContainingBlockBoundingRect;
    auto shiftedPoint = data.OffsetFromContainingBlock;
    shiftedPoint.Scale(-1);
    rect.SetLocation(shiftedPoint);
    return rect;
  }

  Maybe<Path> MotionPath::ComputePathForBox(const BoxPathOperation &,
                                            const TransformOperationData &transformData) noexcept
  {
    if (auto motionPathData = transformData.MotionPathData)
    {
      Path path;
      path.AddRoundedRect(OffsetRectForData(*motionPathData), PathRoundedRect::Strategy::PreferBezier);
      return path;
    }
    return std::nullopt;
  }

  Maybe<Path> MotionPath::ComputePathForShape(const ShapePathOperation &pathOperation,
                                              const TransformOperationData &transformData) noexcept
  {
    if (auto motionPathData = transformData.MotionPathData)
    {
      auto containingBlockRect = OffsetRectForData(*motionPathData).Rect();
      return krys::SwitchOn(
        pathOperation.GetShape(),
        [&]<style::ShapeWithCenterCoordinate T>(const T &shape) -> Maybe<Path>
        {
          if (!shape->Position)
          {
            return style::PathForCenterCoordinate(*shape, containingBlockRect,
                                                  motionPathData->UsedStartingPosition);
          }
          return style::ComputePath(shape, containingBlockRect);
        },
        [&](const auto &shape) -> Maybe<Path> { return style::ComputePath(shape, containingBlockRect); });
    }
    return pathOperation.PathForReferenceRect(transformData.BoundingBox);
  }
}

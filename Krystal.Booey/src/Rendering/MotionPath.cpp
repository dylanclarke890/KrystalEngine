#include "Krystal.Booey/Rendering/MotionPath.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Graphics/PathTraversalState.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Rendering/PathOperation.hpp"
#include "Krystal.Booey/Rendering/TransformOperationData.hpp"
#include "Krystal.Booey/Style/Values/Motion/OffsetPath.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
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

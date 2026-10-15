#pragma once

#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"

namespace krys::boo
{
  class BoxPathOperation;
  class Path;
  class RayPathOperation;
  class ShapePathOperation;
  class TransformationMatrix;
  struct TransformOperationData;

  namespace style
  {
    struct OffsetPath;
  }

  /// @brief Where an element sits relative to the containing block its motion path is resolved against.
  struct MotionPathData
  {
    FloatRoundedRect ContainingBlockBoundingRect;
    FloatPoint OffsetFromContainingBlock;
    FloatPoint UsedStartingPosition;
  };

  /// @brief Resolves `offset-path` operations into paths and applies motion path transforms.
  /// FIXME: `motionPathDataForRenderer` comes with the renderer.
  class MotionPath
  {
  public:
    KRYS_NODISCARD static bool NeedsUpdateAfterContainingBlockLayout(const style::OffsetPath &) noexcept;

    static void ApplyMotionPathTransform(TransformationMatrix &, const TransformOperationData &,
                                         FloatPoint transformOrigin, TransformBox, const Path &,
                                         Maybe<FloatPoint> offsetAnchor, float offsetDistance,
                                         float offsetRotate, bool offsetRotateHasAuto) noexcept;

    KRYS_NODISCARD static Maybe<Path> ComputePathForBox(const BoxPathOperation &,
                                                        const TransformOperationData &) noexcept;
    KRYS_NODISCARD static Maybe<Path> ComputePathForShape(const ShapePathOperation &,
                                                          const TransformOperationData &) noexcept;
    KRYS_NODISCARD static Maybe<Path> ComputePathForRay(const RayPathOperation &,
                                                        const TransformOperationData &) noexcept;
  };
}

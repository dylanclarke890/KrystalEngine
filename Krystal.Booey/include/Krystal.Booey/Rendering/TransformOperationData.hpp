#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Rendering/MotionPath.hpp"

namespace krys::boo
{
  /// @brief What a transform or motion path needs to know about the box it applies to.
  /// FIXME: Construction from a renderer (which fills in the motion path data and whether it's an SVG
  /// renderer).
  struct TransformOperationData
  {
    FloatRect BoundingBox;
    Maybe<boo::MotionPathData> MotionPathData;
    bool IsSVGRenderer {false};

    explicit TransformOperationData(FloatRect boundingBox,
                                    Maybe<boo::MotionPathData> motionPathData = std::nullopt,
                                    bool isSVGRenderer = false) noexcept
        : BoundingBox(boundingBox), MotionPathData(motionPathData), IsSVGRenderer(isSVGRenderer)
    {
    }
  };
}

#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::svg
{
  /// @brief An SVG path command. The values are stored in path byte streams, and match the `SVGPathSeg`
  /// `PATHSEG_*` constants.
  enum class SVGPathSegType : uint8
  {
    Unknown = 0,
    ClosePath = 1,
    MoveToAbs = 2,
    MoveToRel = 3,
    LineToAbs = 4,
    LineToRel = 5,
    CurveToCubicAbs = 6,
    CurveToCubicRel = 7,
    CurveToQuadraticAbs = 8,
    CurveToQuadraticRel = 9,
    ArcAbs = 10,
    ArcRel = 11,
    LineToHorizontalAbs = 12,
    LineToHorizontalRel = 13,
    LineToVerticalAbs = 14,
    LineToVerticalRel = 15,
    CurveToCubicSmoothAbs = 16,
    CurveToCubicSmoothRel = 17,
    CurveToQuadraticSmoothAbs = 18,
    CurveToQuadraticSmoothRel = 19
  };
}

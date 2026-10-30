#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathSegType.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::svg
{
  /// @brief Produces the segments of an SVG path, from a string or a byte stream.
  class SVGPathSource
  {
    KRYS_NON_COPYABLE(SVGPathSource);

  public:
    struct MoveToSegment
    {
      FloatPoint TargetPoint;
    };

    struct LineToSegment
    {
      FloatPoint TargetPoint;
    };

    struct LineToHorizontalSegment
    {
      float X {0};
    };

    struct LineToVerticalSegment
    {
      float Y {0};
    };

    struct CurveToCubicSegment
    {
      FloatPoint Point1;
      FloatPoint Point2;
      FloatPoint TargetPoint;
    };

    struct CurveToCubicSmoothSegment
    {
      FloatPoint Point2;
      FloatPoint TargetPoint;
    };

    struct CurveToQuadraticSegment
    {
      FloatPoint Point1;
      FloatPoint TargetPoint;
    };

    struct CurveToQuadraticSmoothSegment
    {
      FloatPoint TargetPoint;
    };

    struct ArcToSegment
    {
      float Rx {0};
      float Ry {0};
      float Angle {0};
      bool LargeArc {false};
      bool Sweep {false};
      FloatPoint TargetPoint;
    };

    SVGPathSource() noexcept = default;
    virtual ~SVGPathSource() noexcept = default;

    KRYS_NODISCARD virtual bool HasMoreData() const noexcept = 0;
    virtual bool MoveToNextToken() noexcept = 0;
    KRYS_NODISCARD virtual SVGPathSegType NextCommand(SVGPathSegType previousCommand) noexcept = 0;

    KRYS_NODISCARD virtual Maybe<SVGPathSegType> ParseSVGSegmentType() noexcept = 0;

    KRYS_NODISCARD virtual Maybe<MoveToSegment> ParseMoveToSegment(FloatPoint currentPoint) noexcept = 0;
    KRYS_NODISCARD virtual Maybe<LineToSegment> ParseLineToSegment(FloatPoint currentPoint) noexcept = 0;
    KRYS_NODISCARD virtual Maybe<LineToHorizontalSegment>
      ParseLineToHorizontalSegment(FloatPoint currentPoint) noexcept = 0;
    KRYS_NODISCARD virtual Maybe<LineToVerticalSegment>
      ParseLineToVerticalSegment(FloatPoint currentPoint) noexcept = 0;
    KRYS_NODISCARD virtual Maybe<CurveToCubicSegment>
      ParseCurveToCubicSegment(FloatPoint currentPoint) noexcept = 0;
    KRYS_NODISCARD virtual Maybe<CurveToCubicSmoothSegment>
      ParseCurveToCubicSmoothSegment(FloatPoint currentPoint) noexcept = 0;
    KRYS_NODISCARD virtual Maybe<CurveToQuadraticSegment>
      ParseCurveToQuadraticSegment(FloatPoint currentPoint) noexcept = 0;
    KRYS_NODISCARD virtual Maybe<CurveToQuadraticSmoothSegment>
      ParseCurveToQuadraticSmoothSegment(FloatPoint currentPoint) noexcept = 0;
    KRYS_NODISCARD virtual Maybe<ArcToSegment> ParseArcToSegment(FloatPoint currentPoint) noexcept = 0;
  };
}

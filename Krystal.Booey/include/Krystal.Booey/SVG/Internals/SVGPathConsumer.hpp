#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo::svg
{
  enum class PathCoordinateMode : uint8
  {
    AbsoluteCoordinates,
    RelativeCoordinates
  };

  enum class PathParsingMode : uint8
  {
    // Every segment is converted to an absolute move, line, cubic curve or close.
    NormalisedParsing,
    // Segments are passed on as written.
    UnalteredParsing
  };

  /// @brief Receives the segments of a parsed SVG path.
  class SVGPathConsumer
  {
    KRYS_NON_COPYABLE(SVGPathConsumer);

  public:
    SVGPathConsumer() noexcept = default;

    virtual void IncrementPathSegmentCount() noexcept = 0;
    KRYS_NODISCARD virtual bool ContinueConsuming() noexcept = 0;

    // Used in UnalteredParsing/NormalizedParsing modes.
    virtual void MoveTo(const FloatPoint &targetPoint, bool closed, PathCoordinateMode mode) noexcept = 0;
    virtual void LineTo(const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept = 0;
    virtual void CurveToCubic(const FloatPoint &controlPoint1, const FloatPoint &controlPoint2,
                              const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept = 0;
    virtual void ClosePath() noexcept = 0;

    // Only used in UnalteredParsing mode.
    virtual void LineToHorizontal(float x, PathCoordinateMode mode) noexcept = 0;
    virtual void LineToVertical(float y, PathCoordinateMode mode) noexcept = 0;
    virtual void CurveToCubicSmooth(const FloatPoint &controlPoint2, const FloatPoint &targetPoint,
                                    PathCoordinateMode mode) noexcept = 0;
    virtual void CurveToQuadratic(const FloatPoint &controlPoint, const FloatPoint &targetPoint,
                                  PathCoordinateMode mode) noexcept = 0;
    virtual void CurveToQuadraticSmooth(const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept = 0;
    virtual void ArcTo(float r1, float r2, float angle, bool largeArcFlag, bool sweepFlag,
                       const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept = 0;

  protected:
    virtual ~SVGPathConsumer() noexcept = default;
  };
}

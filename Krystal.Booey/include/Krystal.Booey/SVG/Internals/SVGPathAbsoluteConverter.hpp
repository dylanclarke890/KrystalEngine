#pragma once

#include "Krystal.Booey/SVG/Internals/SVGPathConsumer.hpp"

namespace krys::boo::svg
{
  // This consumer translates incoming SVG draw commands into absolute coordinates, and forwards it
  // to another consumer. convertSVGPathByteStreamToAbsoluteCoordinates uses it to convert relative
  // draw commands in an SVG path into absolute.
  /// @brief Forwards parsed SVG path data to another consumer with every segment in absolute coordinates.
  class SVGPathAbsoluteConverter final : public SVGPathConsumer
  {
  public:
    explicit SVGPathAbsoluteConverter(SVGPathConsumer &) noexcept;

  private:
    void IncrementPathSegmentCount() noexcept final;
    KRYS_NODISCARD bool ContinueConsuming() noexcept final;

    // Used in PathParsingMode::UnalteredParsing/PathParsingMode::NormalizedParsing modes.
    void MoveTo(const FloatPoint &targetPoint, bool closed, PathCoordinateMode) noexcept final;
    void LineTo(const FloatPoint &targetPoint, PathCoordinateMode) noexcept final;
    void CurveToCubic(const FloatPoint &point1, const FloatPoint &point2, const FloatPoint &targetPoint,
                      PathCoordinateMode) noexcept final;
    void ClosePath() noexcept final;

    // Only used in PathParsingMode::UnalteredParsing mode.
    void LineToHorizontal(float targetX, PathCoordinateMode) noexcept final;
    void LineToVertical(float targetY, PathCoordinateMode) noexcept final;
    void CurveToCubicSmooth(const FloatPoint &point2, const FloatPoint &targetPoint,
                            PathCoordinateMode) noexcept final;
    void CurveToQuadratic(const FloatPoint &point1, const FloatPoint &targetPoint,
                          PathCoordinateMode) noexcept final;
    void CurveToQuadraticSmooth(const FloatPoint &targetPoint, PathCoordinateMode) noexcept final;
    void ArcTo(float r1, float r2, float angle, bool largeArcFlag, bool sweepFlag,
               const FloatPoint &targetPoint, PathCoordinateMode) noexcept final;

    SVGPathConsumer &_consumer;
    FloatPoint _currentPoint;
    FloatPoint _subpathPoint;
  };
}

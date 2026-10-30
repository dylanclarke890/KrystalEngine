#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathSource.hpp"

namespace krys::boo::svg
{
  /// @brief Reads the segments of SVG path data, e.g. "M 0 0 L 10 10".
  class SVGPathStringViewSource final : public SVGPathSource
  {
  private:
    dom::DOMStringView _buffer;

  public:
    explicit SVGPathStringViewSource(dom::DOMStringView view) noexcept;

  private:
    KRYS_NODISCARD bool HasMoreData() const noexcept final;
    bool MoveToNextToken() noexcept final;
    KRYS_NODISCARD SVGPathSegType NextCommand(SVGPathSegType previousCommand) noexcept final;

    KRYS_NODISCARD Maybe<SVGPathSegType> ParseSVGSegmentType() noexcept final;
    KRYS_NODISCARD Maybe<MoveToSegment> ParseMoveToSegment(FloatPoint currentPoint) noexcept final;
    KRYS_NODISCARD Maybe<LineToSegment> ParseLineToSegment(FloatPoint currentPoint) noexcept final;
    KRYS_NODISCARD Maybe<LineToHorizontalSegment>
      ParseLineToHorizontalSegment(FloatPoint currentPoint) noexcept final;
    KRYS_NODISCARD Maybe<LineToVerticalSegment>
      ParseLineToVerticalSegment(FloatPoint currentPoint) noexcept final;
    KRYS_NODISCARD Maybe<CurveToCubicSegment>
      ParseCurveToCubicSegment(FloatPoint currentPoint) noexcept final;
    KRYS_NODISCARD Maybe<CurveToCubicSmoothSegment>
      ParseCurveToCubicSmoothSegment(FloatPoint currentPoint) noexcept final;
    KRYS_NODISCARD Maybe<CurveToQuadraticSegment>
      ParseCurveToQuadraticSegment(FloatPoint currentPoint) noexcept final;
    KRYS_NODISCARD Maybe<CurveToQuadraticSmoothSegment>
      ParseCurveToQuadraticSmoothSegment(FloatPoint currentPoint) noexcept final;
    KRYS_NODISCARD Maybe<ArcToSegment> ParseArcToSegment(FloatPoint currentPoint) noexcept final;
  };
}

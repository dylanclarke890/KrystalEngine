#include "Krystal.Booey/SVG/Internals/SVGPathStringBuilder.hpp"
#include "Krystal.Booey/Core/NumberFormatting.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::svg
{
  namespace
  {
    void AppendFlag(StringBuilder &stringBuilder, bool flag) noexcept
    {
      stringBuilder.Append(flag ? u8"1 " : u8"0 ");
    }

    void AppendNumber(StringBuilder &stringBuilder, float number) noexcept
    {
      // FIXME: Shortest form would be better, but fixed precision is required for now to smooth over
      // precision errors caused by converting float to double and back in CGPath on Cocoa platforms.
      FormatNumberFixedPrecision(stringBuilder, number);
      stringBuilder.Append(u8' ');
    }

    void AppendPoint(StringBuilder &stringBuilder, const FloatPoint &point) noexcept
    {
      AppendNumber(stringBuilder, point.X());
      AppendNumber(stringBuilder, point.Y());
    }

    void AppendCommand(StringBuilder &stringBuilder, PathCoordinateMode mode, char8_t absolute,
                       char8_t relative) noexcept
    {
      stringBuilder.Append(mode == PathCoordinateMode::AbsoluteCoordinates ? absolute : relative);
      stringBuilder.Append(u8' ');
    }
  }

  SVGPathStringBuilder::SVGPathStringBuilder() noexcept = default;

  SVGPathStringBuilder::~SVGPathStringBuilder() noexcept = default;

  dom::DOMString SVGPathStringBuilder::Result() noexcept
  {
    auto size = _stringBuilder.Length();
    if (!size)
    {
      return {};
    }

    // Remove trailing space.
    _stringBuilder.Shrink(size - 1);
    return _stringBuilder.ToString();
  }

  void SVGPathStringBuilder::IncrementPathSegmentCount() noexcept
  {
  }

  bool SVGPathStringBuilder::ContinueConsuming() noexcept
  {
    return true;
  }

  void SVGPathStringBuilder::MoveTo(const FloatPoint &targetPoint, bool, PathCoordinateMode mode) noexcept
  {
    AppendCommand(_stringBuilder, mode, u8'M', u8'm');
    AppendPoint(_stringBuilder, targetPoint);
  }

  void SVGPathStringBuilder::LineTo(const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept
  {
    AppendCommand(_stringBuilder, mode, u8'L', u8'l');
    AppendPoint(_stringBuilder, targetPoint);
  }

  void SVGPathStringBuilder::LineToHorizontal(float x, PathCoordinateMode mode) noexcept
  {
    AppendCommand(_stringBuilder, mode, u8'H', u8'h');
    AppendNumber(_stringBuilder, x);
  }

  void SVGPathStringBuilder::LineToVertical(float y, PathCoordinateMode mode) noexcept
  {
    AppendCommand(_stringBuilder, mode, u8'V', u8'v');
    AppendNumber(_stringBuilder, y);
  }

  void SVGPathStringBuilder::CurveToCubic(const FloatPoint &point1, const FloatPoint &point2,
                                          const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept
  {
    AppendCommand(_stringBuilder, mode, u8'C', u8'c');
    AppendPoint(_stringBuilder, point1);
    AppendPoint(_stringBuilder, point2);
    AppendPoint(_stringBuilder, targetPoint);
  }

  void SVGPathStringBuilder::CurveToCubicSmooth(const FloatPoint &point2, const FloatPoint &targetPoint,
                                                PathCoordinateMode mode) noexcept
  {
    AppendCommand(_stringBuilder, mode, u8'S', u8's');
    AppendPoint(_stringBuilder, point2);
    AppendPoint(_stringBuilder, targetPoint);
  }

  void SVGPathStringBuilder::CurveToQuadratic(const FloatPoint &point1, const FloatPoint &targetPoint,
                                              PathCoordinateMode mode) noexcept
  {
    AppendCommand(_stringBuilder, mode, u8'Q', u8'q');
    AppendPoint(_stringBuilder, point1);
    AppendPoint(_stringBuilder, targetPoint);
  }

  void SVGPathStringBuilder::CurveToQuadraticSmooth(const FloatPoint &targetPoint,
                                                    PathCoordinateMode mode) noexcept
  {
    AppendCommand(_stringBuilder, mode, u8'T', u8't');
    AppendPoint(_stringBuilder, targetPoint);
  }

  void SVGPathStringBuilder::ArcTo(float r1, float r2, float angle, bool largeArcFlag, bool sweepFlag,
                                   const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept
  {
    AppendCommand(_stringBuilder, mode, u8'A', u8'a');
    AppendNumber(_stringBuilder, r1);
    AppendNumber(_stringBuilder, r2);
    AppendNumber(_stringBuilder, angle);
    AppendFlag(_stringBuilder, largeArcFlag);
    AppendFlag(_stringBuilder, sweepFlag);
    AppendPoint(_stringBuilder, targetPoint);
  }

  void SVGPathStringBuilder::ClosePath() noexcept
  {
    _stringBuilder.Append(u8"Z ");
  }
}

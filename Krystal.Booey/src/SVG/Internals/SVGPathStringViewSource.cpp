#include "Krystal.Booey/SVG/Internals/SVGPathStringViewSource.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::svg
{
  SVGPathStringViewSource::SVGPathStringViewSource(dom::DOMStringView view) noexcept : _buffer(view)
  {
    krys_debug_assert(!view.empty());
  }

  bool SVGPathStringViewSource::HasMoreData() const noexcept
  {
    return !_buffer.empty();
  }

  bool SVGPathStringViewSource::MoveToNextToken() noexcept
  {
    return SkipOptionalSVGSpaces(_buffer);
  }

  SVGPathSegType SVGPathStringViewSource::NextCommand(SVGPathSegType previousCommand) noexcept
  {
    // Check for remaining coordinates in the current command.
    auto c = _buffer.front();
    if ((c == '+' || c == '-' || c == '.' || text::IsASCIIDigit(c))
        && previousCommand != SVGPathSegType::ClosePath)
    {
      if (previousCommand == SVGPathSegType::MoveToAbs)
      {
        return SVGPathSegType::LineToAbs;
      }
      if (previousCommand == SVGPathSegType::MoveToRel)
      {
        return SVGPathSegType::LineToRel;
      }
      return previousCommand;
    }

    return *ParseSVGSegmentType();
  }

  Maybe<SVGPathSegType> SVGPathStringViewSource::ParseSVGSegmentType() noexcept
  {
    auto character = _buffer.front();
    _buffer.remove_prefix(1);
    switch (character)
    {
      case 'Z':
      case 'z': return SVGPathSegType::ClosePath;
      case 'M': return SVGPathSegType::MoveToAbs;
      case 'm': return SVGPathSegType::MoveToRel;
      case 'L': return SVGPathSegType::LineToAbs;
      case 'l': return SVGPathSegType::LineToRel;
      case 'C': return SVGPathSegType::CurveToCubicAbs;
      case 'c': return SVGPathSegType::CurveToCubicRel;
      case 'Q': return SVGPathSegType::CurveToQuadraticAbs;
      case 'q': return SVGPathSegType::CurveToQuadraticRel;
      case 'A': return SVGPathSegType::ArcAbs;
      case 'a': return SVGPathSegType::ArcRel;
      case 'H': return SVGPathSegType::LineToHorizontalAbs;
      case 'h': return SVGPathSegType::LineToHorizontalRel;
      case 'V': return SVGPathSegType::LineToVerticalAbs;
      case 'v': return SVGPathSegType::LineToVerticalRel;
      case 'S': return SVGPathSegType::CurveToCubicSmoothAbs;
      case 's': return SVGPathSegType::CurveToCubicSmoothRel;
      case 'T': return SVGPathSegType::CurveToQuadraticSmoothAbs;
      case 't': return SVGPathSegType::CurveToQuadraticSmoothRel;
      default:  return SVGPathSegType::Unknown;
    }
  }

  Maybe<SVGPathSource::MoveToSegment> SVGPathStringViewSource::ParseMoveToSegment(FloatPoint) noexcept
  {
    auto targetPoint = ParseFloatPoint(_buffer);
    if (!targetPoint)
    {
      return {};
    }

    MoveToSegment segment;
    segment.TargetPoint = *targetPoint;
    return segment;
  }

  Maybe<SVGPathSource::LineToSegment> SVGPathStringViewSource::ParseLineToSegment(FloatPoint) noexcept
  {
    auto targetPoint = ParseFloatPoint(_buffer);
    if (!targetPoint)
    {
      return {};
    }

    LineToSegment segment;
    segment.TargetPoint = *targetPoint;
    return segment;
  }

  Maybe<SVGPathSource::LineToHorizontalSegment>
    SVGPathStringViewSource::ParseLineToHorizontalSegment(FloatPoint) noexcept
  {
    auto x = ParseNumber(_buffer);
    if (!x)
    {
      return {};
    }

    LineToHorizontalSegment segment;
    segment.X = *x;
    return segment;
  }

  Maybe<SVGPathSource::LineToVerticalSegment>
    SVGPathStringViewSource::ParseLineToVerticalSegment(FloatPoint) noexcept
  {
    auto y = ParseNumber(_buffer);
    if (!y)
    {
      return {};
    }

    LineToVerticalSegment segment;
    segment.Y = *y;
    return segment;
  }

  Maybe<SVGPathSource::CurveToCubicSegment>
    SVGPathStringViewSource::ParseCurveToCubicSegment(FloatPoint) noexcept
  {
    auto point1 = ParseFloatPoint(_buffer);
    if (!point1)
    {
      return {};
    }

    auto point2 = ParseFloatPoint(_buffer);
    if (!point2)
    {
      return {};
    }

    auto targetPoint = ParseFloatPoint(_buffer);
    if (!targetPoint)
    {
      return {};
    }

    CurveToCubicSegment segment;
    segment.Point1 = *point1;
    segment.Point2 = *point2;
    segment.TargetPoint = *targetPoint;
    return segment;
  }

  Maybe<SVGPathSource::CurveToCubicSmoothSegment>
    SVGPathStringViewSource::ParseCurveToCubicSmoothSegment(FloatPoint) noexcept
  {
    auto point2 = ParseFloatPoint(_buffer);
    if (!point2)
    {
      return {};
    }

    auto targetPoint = ParseFloatPoint(_buffer);
    if (!targetPoint)
    {
      return {};
    }

    CurveToCubicSmoothSegment segment;
    segment.Point2 = *point2;
    segment.TargetPoint = *targetPoint;
    return segment;
  }

  Maybe<SVGPathSource::CurveToQuadraticSegment>
    SVGPathStringViewSource::ParseCurveToQuadraticSegment(FloatPoint) noexcept
  {
    auto point1 = ParseFloatPoint(_buffer);
    if (!point1)
    {
      return {};
    }

    auto targetPoint = ParseFloatPoint(_buffer);
    if (!targetPoint)
    {
      return {};
    }

    CurveToQuadraticSegment segment;
    segment.Point1 = *point1;
    segment.TargetPoint = *targetPoint;
    return segment;
  }

  Maybe<SVGPathSource::CurveToQuadraticSmoothSegment>
    SVGPathStringViewSource::ParseCurveToQuadraticSmoothSegment(FloatPoint) noexcept
  {
    auto targetPoint = ParseFloatPoint(_buffer);
    if (!targetPoint)
    {
      return {};
    }

    CurveToQuadraticSmoothSegment segment;
    segment.TargetPoint = *targetPoint;
    return segment;
  }

  Maybe<SVGPathSource::ArcToSegment> SVGPathStringViewSource::ParseArcToSegment(FloatPoint) noexcept
  {
    auto rx = ParseNumber(_buffer);
    if (!rx)
    {
      return {};
    }
    auto ry = ParseNumber(_buffer);
    if (!ry)
    {
      return {};
    }
    auto angle = ParseNumber(_buffer);
    if (!angle)
    {
      return {};
    }
    auto largeArc = ParseArcFlag(_buffer);
    if (!largeArc)
    {
      return {};
    }
    auto sweep = ParseArcFlag(_buffer);
    if (!sweep)
    {
      return {};
    }
    auto targetPoint = ParseFloatPoint(_buffer);
    if (!targetPoint)
    {
      return {};
    }

    ArcToSegment segment;
    segment.Rx = *rx;
    segment.Ry = *ry;
    segment.Angle = *angle;
    segment.LargeArc = *largeArc;
    segment.Sweep = *sweep;
    segment.TargetPoint = *targetPoint;
    return segment;
  }
}

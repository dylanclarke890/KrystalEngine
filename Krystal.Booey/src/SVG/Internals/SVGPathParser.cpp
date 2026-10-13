#include "Krystal.Booey/SVG/Internals/SVGPathParser.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathByteStreamBuilder.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathSource.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathStringBuilder.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <cmath>
#include <numbers>

namespace krys::boo::svg
{
  namespace
  {
    constexpr float OneOverThree = 1 / 3.f;

    KRYS_NODISCARD constexpr FloatPoint ReflectedPoint(const FloatPoint &center,
                                                       const FloatPoint &point) noexcept
    {
      // `center` is the middle point between `point` and `reflectedPoint`.
      return {2 * center.X() - point.X(), 2 * center.Y() - point.Y()};
    }
  }

  bool SVGPathParser::Parse(SVGPathSource &source, SVGPathConsumer &consumer, PathParsingMode mode,
                            bool checkForInitialMoveTo) noexcept
  {
    SVGPathParser parser(consumer, source, mode);
    return parser.ParsePathData(checkForInitialMoveTo);
  }

  bool SVGPathParser::ParseToByteStream(SVGPathSource &source, SVGPathByteStream &byteStream,
                                        PathParsingMode mode, bool checkForInitialMoveTo) noexcept
  {
    SVGPathByteStreamBuilder builder(byteStream);
    auto result = Parse(source, builder, mode, checkForInitialMoveTo);
    byteStream.ShrinkToFit();
    return result;
  }

  bool SVGPathParser::ParseToString(SVGPathSource &source, dom::DOMString &result, PathParsingMode mode,
                                    bool checkForInitialMoveTo) noexcept
  {
    SVGPathStringBuilder builder;
    SVGPathParser parser(builder, source, mode);
    bool ok = parser.ParsePathData(checkForInitialMoveTo);
    result = builder.Result();
    return ok;
  }

  SVGPathParser::SVGPathParser(SVGPathConsumer &consumer, SVGPathSource &source,
                               PathParsingMode parsingMode) noexcept
      : _source(source), _consumer(consumer), _pathParsingMode(parsingMode)
  {
  }

  void SVGPathParser::ParseClosePathSegment() noexcept
  {
    // Reset _currentPoint for the next path.
    if (_pathParsingMode == PathParsingMode::NormalisedParsing)
    {
      _currentPoint = _subPathPoint;
    }
    _closePath = true;
    _consumer.ClosePath();
  }

  bool SVGPathParser::ParseMoveToSegment() noexcept
  {
    auto result = _source.ParseMoveToSegment(_currentPoint);
    if (!result)
    {
      return false;
    }

    if (_pathParsingMode == PathParsingMode::NormalisedParsing)
    {
      if (_mode == PathCoordinateMode::RelativeCoordinates)
      {
        _currentPoint += result->TargetPoint;
      }
      else
      {
        _currentPoint = result->TargetPoint;
      }
      _subPathPoint = _currentPoint;
      _consumer.MoveTo(_currentPoint, _closePath, PathCoordinateMode::AbsoluteCoordinates);
    }
    else
    {
      _consumer.MoveTo(result->TargetPoint, _closePath, _mode);
    }
    _closePath = false;
    return true;
  }

  bool SVGPathParser::ParseLineToSegment() noexcept
  {
    auto result = _source.ParseLineToSegment(_currentPoint);
    if (!result)
    {
      return false;
    }

    if (_pathParsingMode == PathParsingMode::NormalisedParsing)
    {
      if (_mode == PathCoordinateMode::RelativeCoordinates)
      {
        _currentPoint += result->TargetPoint;
      }
      else
      {
        _currentPoint = result->TargetPoint;
      }
      _consumer.LineTo(_currentPoint, PathCoordinateMode::AbsoluteCoordinates);
    }
    else
    {
      _consumer.LineTo(result->TargetPoint, _mode);
    }
    return true;
  }

  bool SVGPathParser::ParseLineToHorizontalSegment() noexcept
  {
    auto result = _source.ParseLineToHorizontalSegment(_currentPoint);
    if (!result)
    {
      return false;
    }

    if (_pathParsingMode == PathParsingMode::NormalisedParsing)
    {
      if (_mode == PathCoordinateMode::RelativeCoordinates)
      {
        _currentPoint.Move(result->X, 0);
      }
      else
      {
        _currentPoint.SetX(result->X);
      }
      _consumer.LineTo(_currentPoint, PathCoordinateMode::AbsoluteCoordinates);
    }
    else
    {
      _consumer.LineToHorizontal(result->X, _mode);
    }
    return true;
  }

  bool SVGPathParser::ParseLineToVerticalSegment() noexcept
  {
    auto result = _source.ParseLineToVerticalSegment(_currentPoint);
    if (!result)
    {
      return false;
    }

    if (_pathParsingMode == PathParsingMode::NormalisedParsing)
    {
      if (_mode == PathCoordinateMode::RelativeCoordinates)
      {
        _currentPoint.Move(0, result->Y);
      }
      else
      {
        _currentPoint.SetY(result->Y);
      }
      _consumer.LineTo(_currentPoint, PathCoordinateMode::AbsoluteCoordinates);
    }
    else
    {
      _consumer.LineToVertical(result->Y, _mode);
    }
    return true;
  }

  bool SVGPathParser::ParseCurveToCubicSegment() noexcept
  {
    auto result = _source.ParseCurveToCubicSegment(_currentPoint);
    if (!result)
    {
      return false;
    }

    if (_pathParsingMode == PathParsingMode::NormalisedParsing)
    {
      if (_mode == PathCoordinateMode::RelativeCoordinates)
      {
        result->Point1 += _currentPoint;
        result->Point2 += _currentPoint;
        result->TargetPoint += _currentPoint;
      }
      _consumer.CurveToCubic(result->Point1, result->Point2, result->TargetPoint,
                             PathCoordinateMode::AbsoluteCoordinates);

      _controlPoint = result->Point2;
      _currentPoint = result->TargetPoint;
    }
    else
    {
      _consumer.CurveToCubic(result->Point1, result->Point2, result->TargetPoint, _mode);
    }
    return true;
  }

  bool SVGPathParser::ParseCurveToCubicSmoothSegment() noexcept
  {
    auto result = _source.ParseCurveToCubicSmoothSegment(_currentPoint);
    if (!result)
    {
      return false;
    }

    if (_lastCommand != SVGPathSegType::CurveToCubicAbs && _lastCommand != SVGPathSegType::CurveToCubicRel
        && _lastCommand != SVGPathSegType::CurveToCubicSmoothAbs
        && _lastCommand != SVGPathSegType::CurveToCubicSmoothRel)
    {
      _controlPoint = _currentPoint;
    }

    if (_pathParsingMode == PathParsingMode::NormalisedParsing)
    {
      FloatPoint point1 = ReflectedPoint(_currentPoint, _controlPoint);
      if (_mode == PathCoordinateMode::RelativeCoordinates)
      {
        result->Point2 += _currentPoint;
        result->TargetPoint += _currentPoint;
      }

      _consumer.CurveToCubic(point1, result->Point2, result->TargetPoint,
                             PathCoordinateMode::AbsoluteCoordinates);

      _controlPoint = result->Point2;
      _currentPoint = result->TargetPoint;
    }
    else
    {
      _consumer.CurveToCubicSmooth(result->Point2, result->TargetPoint, _mode);
    }
    return true;
  }

  bool SVGPathParser::ParseCurveToQuadraticSegment() noexcept
  {
    auto result = _source.ParseCurveToQuadraticSegment(_currentPoint);
    if (!result)
    {
      return false;
    }

    if (_pathParsingMode == PathParsingMode::NormalisedParsing)
    {
      _controlPoint = result->Point1;

      FloatPoint point1 = _currentPoint;
      point1.Move(2 * _controlPoint.X(), 2 * _controlPoint.Y());
      FloatPoint point2(result->TargetPoint.X() + 2 * _controlPoint.X(),
                        result->TargetPoint.Y() + 2 * _controlPoint.Y());
      if (_mode == PathCoordinateMode::RelativeCoordinates)
      {
        point1.Move(2 * _currentPoint.X(), 2 * _currentPoint.Y());
        point2.Move(3 * _currentPoint.X(), 3 * _currentPoint.Y());
        result->TargetPoint += _currentPoint;
      }
      point1.Scale(OneOverThree);
      point2.Scale(OneOverThree);

      _consumer.CurveToCubic(point1, point2, result->TargetPoint, PathCoordinateMode::AbsoluteCoordinates);

      if (_mode == PathCoordinateMode::RelativeCoordinates)
      {
        _controlPoint += _currentPoint;
      }
      _currentPoint = result->TargetPoint;
    }
    else
    {
      _consumer.CurveToQuadratic(result->Point1, result->TargetPoint, _mode);
    }
    return true;
  }

  bool SVGPathParser::ParseCurveToQuadraticSmoothSegment() noexcept
  {
    auto result = _source.ParseCurveToQuadraticSmoothSegment(_currentPoint);
    if (!result)
    {
      return false;
    }

    if (_lastCommand != SVGPathSegType::CurveToQuadraticAbs
        && _lastCommand != SVGPathSegType::CurveToQuadraticRel
        && _lastCommand != SVGPathSegType::CurveToQuadraticSmoothAbs
        && _lastCommand != SVGPathSegType::CurveToQuadraticSmoothRel)
    {
      _controlPoint = _currentPoint;
    }

    if (_pathParsingMode == PathParsingMode::NormalisedParsing)
    {
      FloatPoint cubicPoint = ReflectedPoint(_currentPoint, _controlPoint);
      FloatPoint point1(_currentPoint.X() + 2 * cubicPoint.X(), _currentPoint.Y() + 2 * cubicPoint.Y());
      FloatPoint point2(result->TargetPoint.X() + 2 * cubicPoint.X(),
                        result->TargetPoint.Y() + 2 * cubicPoint.Y());
      if (_mode == PathCoordinateMode::RelativeCoordinates)
      {
        point2 += _currentPoint;
        result->TargetPoint += _currentPoint;
      }
      point1.Scale(OneOverThree);
      point2.Scale(OneOverThree);

      _consumer.CurveToCubic(point1, point2, result->TargetPoint, PathCoordinateMode::AbsoluteCoordinates);

      _controlPoint = cubicPoint;
      _currentPoint = result->TargetPoint;
    }
    else
    {
      _consumer.CurveToQuadraticSmooth(result->TargetPoint, _mode);
    }
    return true;
  }

  bool SVGPathParser::ParseArcToSegment() noexcept
  {
    auto result = _source.ParseArcToSegment(_currentPoint);
    if (!result)
    {
      return false;
    }

    // If rx = 0 or ry = 0 then this arc is treated as a straight line segment (a "lineto") joining the
    // endpoints. http://www.w3.org/TR/SVG/implnote.html#ArcOutOfRangeParameters If the current point and
    // target point for the arc are identical, it should be treated as a zero length path. This ensures
    // continuity in animations.
    bool arcIsZeroLength = false;
    if (_pathParsingMode == PathParsingMode::NormalisedParsing)
    {
      result->Rx = std::abs(result->Rx);
      result->Ry = std::abs(result->Ry);
      if (_mode == PathCoordinateMode::RelativeCoordinates)
      {
        arcIsZeroLength = result->TargetPoint == FloatPoint::Zero();
      }
      else
      {
        arcIsZeroLength = result->TargetPoint == _currentPoint;
      }
    }
    if (!result->Rx || !result->Ry || arcIsZeroLength)
    {
      if (_pathParsingMode == PathParsingMode::NormalisedParsing)
      {
        if (_mode == PathCoordinateMode::RelativeCoordinates)
        {
          _currentPoint += result->TargetPoint;
        }
        else
        {
          _currentPoint = result->TargetPoint;
        }
        _consumer.LineTo(_currentPoint, PathCoordinateMode::AbsoluteCoordinates);
      }
      else
      {
        _consumer.LineTo(result->TargetPoint, _mode);
      }
      return true;
    }

    if (_pathParsingMode == PathParsingMode::NormalisedParsing)
    {
      FloatPoint point1 = _currentPoint;
      if (_mode == PathCoordinateMode::RelativeCoordinates)
      {
        result->TargetPoint += _currentPoint;
      }
      _currentPoint = result->TargetPoint;
      return DecomposeArcToCubic(result->Angle, result->Rx, result->Ry, point1, result->TargetPoint,
                                 result->LargeArc, result->Sweep);
    }

    _consumer.ArcTo(result->Rx, result->Ry, result->Angle, result->LargeArc, result->Sweep,
                    result->TargetPoint, _mode);
    return true;
  }

  bool SVGPathParser::ParsePathData(bool checkForInitialMoveTo) noexcept
  {
    // Skip any leading spaces.
    if (!_source.MoveToNextToken())
    {
      return true;
    }

    auto parsedCommand = _source.ParseSVGSegmentType();
    if (!parsedCommand)
    {
      return false;
    }

    auto command = *parsedCommand;

    // Path must start with moveto.
    if (checkForInitialMoveTo && command != SVGPathSegType::MoveToAbs && command != SVGPathSegType::MoveToRel)
    {
      return false;
    }

    while (true)
    {
      // Skip spaces between command and first coordinate.
      _source.MoveToNextToken();
      _mode = PathCoordinateMode::AbsoluteCoordinates;
      switch (command)
      {
        case SVGPathSegType::MoveToRel: _mode = PathCoordinateMode::RelativeCoordinates; [[fallthrough]];
        case SVGPathSegType::MoveToAbs:
          if (!ParseMoveToSegment())
          {
            return false;
          }
          break;
        case SVGPathSegType::LineToRel: _mode = PathCoordinateMode::RelativeCoordinates; [[fallthrough]];
        case SVGPathSegType::LineToAbs:
          if (!ParseLineToSegment())
          {
            return false;
          }
          break;
        case SVGPathSegType::LineToHorizontalRel:
          _mode = PathCoordinateMode::RelativeCoordinates;
          [[fallthrough]];
        case SVGPathSegType::LineToHorizontalAbs:
          if (!ParseLineToHorizontalSegment())
          {
            return false;
          }
          break;
        case SVGPathSegType::LineToVerticalRel:
          _mode = PathCoordinateMode::RelativeCoordinates;
          [[fallthrough]];
        case SVGPathSegType::LineToVerticalAbs:
          if (!ParseLineToVerticalSegment())
          {
            return false;
          }
          break;
        case SVGPathSegType::ClosePath: ParseClosePathSegment(); break;
        case SVGPathSegType::CurveToCubicRel:
          _mode = PathCoordinateMode::RelativeCoordinates;
          [[fallthrough]];
        case SVGPathSegType::CurveToCubicAbs:
          if (!ParseCurveToCubicSegment())
          {
            return false;
          }
          break;
        case SVGPathSegType::CurveToCubicSmoothRel:
          _mode = PathCoordinateMode::RelativeCoordinates;
          [[fallthrough]];
        case SVGPathSegType::CurveToCubicSmoothAbs:
          if (!ParseCurveToCubicSmoothSegment())
          {
            return false;
          }
          break;
        case SVGPathSegType::CurveToQuadraticRel:
          _mode = PathCoordinateMode::RelativeCoordinates;
          [[fallthrough]];
        case SVGPathSegType::CurveToQuadraticAbs:
          if (!ParseCurveToQuadraticSegment())
          {
            return false;
          }
          break;
        case SVGPathSegType::CurveToQuadraticSmoothRel:
          _mode = PathCoordinateMode::RelativeCoordinates;
          [[fallthrough]];
        case SVGPathSegType::CurveToQuadraticSmoothAbs:
          if (!ParseCurveToQuadraticSmoothSegment())
          {
            return false;
          }
          break;
        case SVGPathSegType::ArcRel: _mode = PathCoordinateMode::RelativeCoordinates; [[fallthrough]];
        case SVGPathSegType::ArcAbs:
          if (!ParseArcToSegment())
          {
            return false;
          }
          break;
        default: return false;
      }
      if (!_consumer.ContinueConsuming())
      {
        return true;
      }

      _lastCommand = command;

      if (!_source.HasMoreData())
      {
        return true;
      }

      command = _source.NextCommand(command);

      if (_lastCommand != SVGPathSegType::CurveToCubicAbs && _lastCommand != SVGPathSegType::CurveToCubicRel
          && _lastCommand != SVGPathSegType::CurveToCubicSmoothAbs
          && _lastCommand != SVGPathSegType::CurveToCubicSmoothRel
          && _lastCommand != SVGPathSegType::CurveToQuadraticAbs
          && _lastCommand != SVGPathSegType::CurveToQuadraticRel
          && _lastCommand != SVGPathSegType::CurveToQuadraticSmoothAbs
          && _lastCommand != SVGPathSegType::CurveToQuadraticSmoothRel)
      {
        _controlPoint = _currentPoint;
      }

      _consumer.IncrementPathSegmentCount();
    }

    return false;
  }

  // This works by converting the SVG arc to "simple" beziers.
  // Partly adapted from Niko's code in kdelibs/kdecore/svgicons.
  // See also SVG implementation notes: http://www.w3.org/TR/SVG/implnote.html#ArcConversionEndpointToCenter
  bool SVGPathParser::DecomposeArcToCubic(float angle, float rx, float ry, const FloatPoint &start,
                                          const FloatPoint &end, bool largeArcFlag, bool sweepFlag) noexcept
  {
    FloatSize midPointDistance = start - end;
    midPointDistance.Scale(0.5f);

    AffineTransform pointTransform;
    pointTransform.Rotate(-angle);

    FloatPoint transformedMidPoint =
      pointTransform.MapPoint(FloatPoint(midPointDistance.Width(), midPointDistance.Height()));
    float squareRx = rx * rx;
    float squareRy = ry * ry;
    float squareX = transformedMidPoint.X() * transformedMidPoint.X();
    float squareY = transformedMidPoint.Y() * transformedMidPoint.Y();

    // Check if the radii are big enough to draw the arc, scale radii if not.
    // http://www.w3.org/TR/SVG/implnote.html#ArcCorrectionOutOfRangeRadii
    float radiiScale = squareX / squareRx + squareY / squareRy;
    if (radiiScale > 1)
    {
      rx *= std::sqrt(radiiScale);
      ry *= std::sqrt(radiiScale);
    }

    pointTransform.MakeIdentity();
    pointTransform.Scale(1 / rx, 1 / ry);
    pointTransform.Rotate(-angle);

    FloatPoint point1 = pointTransform.MapPoint(start);
    FloatPoint point2 = pointTransform.MapPoint(end);
    FloatSize delta = point2 - point1;

    float d = delta.Width() * delta.Width() + delta.Height() * delta.Height();
    float scaleFactorSquared = std::max(1 / d - 0.25f, 0.f);

    float scaleFactor = std::sqrt(scaleFactorSquared);
    if (sweepFlag == largeArcFlag)
    {
      scaleFactor = -scaleFactor;
    }

    delta.Scale(scaleFactor);
    FloatPoint centerPoint = point1 + point2;
    centerPoint.Scale(0.5f);
    centerPoint.Move(-delta.Height(), delta.Width());

    float theta1 = FloatPoint(point1 - centerPoint).SlopeAngleRadians();
    float theta2 = FloatPoint(point2 - centerPoint).SlopeAngleRadians();

    float thetaArc = theta2 - theta1;
    if (thetaArc < 0 && sweepFlag)
    {
      thetaArc += 2 * std::numbers::pi_v<float>;
    }
    else if (thetaArc > 0 && !sweepFlag)
    {
      thetaArc -= 2 * std::numbers::pi_v<float>;
    }

    pointTransform.MakeIdentity();
    pointTransform.Rotate(angle);
    pointTransform.Scale(rx, ry);

    // Some results of atan2 on some platform implementations are not exact enough. So that we get more
    // cubic curves than expected here. Adding 0.001f reduces the count of sgements to the correct count.
    int segments = static_cast<int>(std::ceil(std::abs(thetaArc / (piOverTwoFloat + 0.001f))));
    for (int i = 0; i < segments; ++i)
    {
      float startTheta = theta1 + i * thetaArc / segments;
      float endTheta = theta1 + (i + 1) * thetaArc / segments;

      float t = (8 / 6.f) * std::tan(0.25f * (endTheta - startTheta));
      if (!std::isfinite(t))
      {
        return false;
      }
      float sinStartTheta = std::sin(startTheta);
      float cosStartTheta = std::cos(startTheta);
      float sinEndTheta = std::sin(endTheta);
      float cosEndTheta = std::cos(endTheta);

      point1 = FloatPoint(cosStartTheta - t * sinStartTheta, sinStartTheta + t * cosStartTheta);
      point1.Move(centerPoint.X(), centerPoint.Y());
      FloatPoint targetPoint = FloatPoint(cosEndTheta, sinEndTheta);
      targetPoint.Move(centerPoint.X(), centerPoint.Y());
      point2 = targetPoint;
      point2.Move(t * sinEndTheta, -t * cosEndTheta);

      _consumer.CurveToCubic(pointTransform.MapPoint(point1), pointTransform.MapPoint(point2),
                             pointTransform.MapPoint(targetPoint), PathCoordinateMode::AbsoluteCoordinates);
    }
    return true;
  }
}

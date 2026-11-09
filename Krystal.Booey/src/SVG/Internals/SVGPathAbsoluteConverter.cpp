/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/SVG/Internals/SVGPathAbsoluteConverter.hpp"

namespace krys::boo::svg
{
  SVGPathAbsoluteConverter::SVGPathAbsoluteConverter(SVGPathConsumer &consumer) noexcept
      : SVGPathConsumer(), _consumer(consumer)
  {
  }

  void SVGPathAbsoluteConverter::IncrementPathSegmentCount() noexcept
  {
    _consumer.IncrementPathSegmentCount();
  }

  bool SVGPathAbsoluteConverter::ContinueConsuming() noexcept
  {
    return _consumer.ContinueConsuming();
  }

  void SVGPathAbsoluteConverter::MoveTo(const FloatPoint &targetPoint, bool closed,
                                        PathCoordinateMode mode) noexcept
  {
    if (mode == PathCoordinateMode::AbsoluteCoordinates)
    {
      _consumer.MoveTo(targetPoint, closed, PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint = targetPoint;
    }
    else
    {
      _consumer.MoveTo(_currentPoint + targetPoint, closed, PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint += targetPoint;
    }

    _subpathPoint = _currentPoint;
  }

  void SVGPathAbsoluteConverter::LineTo(const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept
  {
    if (mode == PathCoordinateMode::AbsoluteCoordinates)
    {
      _consumer.LineTo(targetPoint, PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint = targetPoint;
    }
    else
    {
      _consumer.LineTo(_currentPoint + targetPoint, PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint += targetPoint;
    }
  }

  void SVGPathAbsoluteConverter::CurveToCubic(const FloatPoint &point1, const FloatPoint &point2,
                                              const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept
  {
    if (mode == PathCoordinateMode::AbsoluteCoordinates)
    {
      _consumer.CurveToCubic(point1, point2, targetPoint, PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint = targetPoint;
    }
    else
    {
      _consumer.CurveToCubic(_currentPoint + point1, _currentPoint + point2, _currentPoint + targetPoint,
                             PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint += targetPoint;
    }
  }

  void SVGPathAbsoluteConverter::ClosePath() noexcept
  {
    _consumer.ClosePath();
    _currentPoint = _subpathPoint;
  }

  void SVGPathAbsoluteConverter::LineToHorizontal(float targetX, PathCoordinateMode mode) noexcept
  {
    if (mode == PathCoordinateMode::AbsoluteCoordinates)
    {
      _consumer.LineToHorizontal(targetX, PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint.SetX(targetX);
    }
    else
    {
      auto absoluteTargetX = _currentPoint.X() + targetX;

      _consumer.LineToHorizontal(absoluteTargetX, PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint.SetX(absoluteTargetX);
    }
  }

  void SVGPathAbsoluteConverter::LineToVertical(float targetY, PathCoordinateMode mode) noexcept
  {
    if (mode == PathCoordinateMode::AbsoluteCoordinates)
    {
      _consumer.LineToVertical(targetY, PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint.SetY(targetY);
    }
    else
    {
      auto absoluteTargetY = _currentPoint.Y() + targetY;

      _consumer.LineToVertical(absoluteTargetY, PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint.SetY(absoluteTargetY);
    }
  }

  void SVGPathAbsoluteConverter::CurveToCubicSmooth(const FloatPoint &point2, const FloatPoint &targetPoint,
                                                    PathCoordinateMode mode) noexcept
  {
    if (mode == PathCoordinateMode::AbsoluteCoordinates)
    {
      _consumer.CurveToCubicSmooth(point2, targetPoint, PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint = targetPoint;
    }
    else
    {
      _consumer.CurveToCubicSmooth(_currentPoint + point2, _currentPoint + targetPoint,
                                   PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint += targetPoint;
    }
  }

  void SVGPathAbsoluteConverter::CurveToQuadratic(const FloatPoint &point1, const FloatPoint &targetPoint,
                                                  PathCoordinateMode mode) noexcept
  {
    if (mode == PathCoordinateMode::AbsoluteCoordinates)
    {
      _consumer.CurveToQuadratic(point1, targetPoint, PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint = targetPoint;
    }
    else
    {
      _consumer.CurveToQuadratic(_currentPoint + point1, _currentPoint + targetPoint,
                                 PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint += targetPoint;
    }
  }

  void SVGPathAbsoluteConverter::CurveToQuadraticSmooth(const FloatPoint &targetPoint,
                                                        PathCoordinateMode mode) noexcept
  {
    if (mode == PathCoordinateMode::AbsoluteCoordinates)
    {
      _consumer.CurveToQuadraticSmooth(targetPoint, PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint = targetPoint;
    }
    else
    {
      _consumer.CurveToQuadraticSmooth(_currentPoint + targetPoint, PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint += targetPoint;
    }
  }

  void SVGPathAbsoluteConverter::ArcTo(float r1, float r2, float angle, bool largeArcFlag, bool sweepFlag,
                                       const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept
  {
    if (mode == PathCoordinateMode::AbsoluteCoordinates)
    {
      _consumer.ArcTo(r1, r2, angle, largeArcFlag, sweepFlag, targetPoint,
                      PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint = targetPoint;
    }
    else
    {
      _consumer.ArcTo(r1, r2, angle, largeArcFlag, sweepFlag, _currentPoint + targetPoint,
                      PathCoordinateMode::AbsoluteCoordinates);
      _currentPoint += targetPoint;
    }
  }
}

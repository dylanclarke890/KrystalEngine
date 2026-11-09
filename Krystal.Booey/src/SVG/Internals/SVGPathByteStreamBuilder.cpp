/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/SVG/Internals/SVGPathByteStreamBuilder.hpp"

namespace krys::boo::svg
{
  SVGPathByteStreamBuilder::SVGPathByteStreamBuilder(SVGPathByteStream &byteStream) noexcept
      : _byteStream(byteStream)
  {
  }

  void SVGPathByteStreamBuilder::MoveTo(const FloatPoint &targetPoint, bool, PathCoordinateMode mode) noexcept
  {
    WriteSegmentType(mode == PathCoordinateMode::RelativeCoordinates ? SVGPathSegType::MoveToRel
                                                                     : SVGPathSegType::MoveToAbs);
    WriteType(targetPoint);
  }

  void SVGPathByteStreamBuilder::LineTo(const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept
  {
    WriteSegmentType(mode == PathCoordinateMode::RelativeCoordinates ? SVGPathSegType::LineToRel
                                                                     : SVGPathSegType::LineToAbs);
    WriteType(targetPoint);
  }

  void SVGPathByteStreamBuilder::LineToHorizontal(float x, PathCoordinateMode mode) noexcept
  {
    WriteSegmentType(mode == PathCoordinateMode::RelativeCoordinates ? SVGPathSegType::LineToHorizontalRel
                                                                     : SVGPathSegType::LineToHorizontalAbs);
    WriteType(x);
  }

  void SVGPathByteStreamBuilder::LineToVertical(float y, PathCoordinateMode mode) noexcept
  {
    WriteSegmentType(mode == PathCoordinateMode::RelativeCoordinates ? SVGPathSegType::LineToVerticalRel
                                                                     : SVGPathSegType::LineToVerticalAbs);
    WriteType(y);
  }

  void SVGPathByteStreamBuilder::CurveToCubic(const FloatPoint &point1, const FloatPoint &point2,
                                              const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept
  {
    WriteSegmentType(mode == PathCoordinateMode::RelativeCoordinates ? SVGPathSegType::CurveToCubicRel
                                                                     : SVGPathSegType::CurveToCubicAbs);
    WriteType(point1);
    WriteType(point2);
    WriteType(targetPoint);
  }

  void SVGPathByteStreamBuilder::CurveToCubicSmooth(const FloatPoint &point2, const FloatPoint &targetPoint,
                                                    PathCoordinateMode mode) noexcept
  {
    WriteSegmentType(mode == PathCoordinateMode::RelativeCoordinates ? SVGPathSegType::CurveToCubicSmoothRel
                                                                     : SVGPathSegType::CurveToCubicSmoothAbs);
    WriteType(point2);
    WriteType(targetPoint);
  }

  void SVGPathByteStreamBuilder::CurveToQuadratic(const FloatPoint &point1, const FloatPoint &targetPoint,
                                                  PathCoordinateMode mode) noexcept
  {
    WriteSegmentType(mode == PathCoordinateMode::RelativeCoordinates ? SVGPathSegType::CurveToQuadraticRel
                                                                     : SVGPathSegType::CurveToQuadraticAbs);
    WriteType(point1);
    WriteType(targetPoint);
  }

  void SVGPathByteStreamBuilder::CurveToQuadraticSmooth(const FloatPoint &targetPoint,
                                                        PathCoordinateMode mode) noexcept
  {
    WriteSegmentType(mode == PathCoordinateMode::RelativeCoordinates
                       ? SVGPathSegType::CurveToQuadraticSmoothRel
                       : SVGPathSegType::CurveToQuadraticSmoothAbs);
    WriteType(targetPoint);
  }

  void SVGPathByteStreamBuilder::ArcTo(float r1, float r2, float angle, bool largeArcFlag, bool sweepFlag,
                                       const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept
  {
    WriteSegmentType(mode == PathCoordinateMode::RelativeCoordinates ? SVGPathSegType::ArcRel
                                                                     : SVGPathSegType::ArcAbs);
    WriteType(r1);
    WriteType(r2);
    WriteType(angle);
    WriteType(largeArcFlag);
    WriteType(sweepFlag);
    WriteType(targetPoint);
  }

  void SVGPathByteStreamBuilder::ClosePath() noexcept
  {
    WriteSegmentType(SVGPathSegType::ClosePath);
  }
}

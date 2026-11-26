/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#include "Krystal.Web/Svg/Internals/SVGPathByteStreamSource.hpp"

namespace krys::web
{
  SVGPathByteStreamSource::SVGPathByteStreamSource(const SVGPathByteStream &stream) noexcept
      : _streamCurrent(stream.GetBytes())
  {
  }

  bool SVGPathByteStreamSource::HasMoreData() const noexcept
  {
    return !_streamCurrent.empty();
  }

  SVGPathSegType SVGPathByteStreamSource::NextCommand(SVGPathSegType) noexcept
  {
    return ReadSVGSegmentType();
  }

  Maybe<SVGPathSegType> SVGPathByteStreamSource::ParseSVGSegmentType() noexcept
  {
    return ReadSVGSegmentType();
  }

  Maybe<SVGPathSource::MoveToSegment> SVGPathByteStreamSource::ParseMoveToSegment(PointF) noexcept
  {
    MoveToSegment segment;
    segment.TargetPoint = ReadPointF();
    return segment;
  }

  Maybe<SVGPathSource::LineToSegment> SVGPathByteStreamSource::ParseLineToSegment(PointF) noexcept
  {
    LineToSegment segment;
    segment.TargetPoint = ReadPointF();
    return segment;
  }

  Maybe<SVGPathSource::LineToHorizontalSegment>
    SVGPathByteStreamSource::ParseLineToHorizontalSegment(PointF) noexcept
  {
    LineToHorizontalSegment segment;
    segment.X = ReadFloat();
    return segment;
  }

  Maybe<SVGPathSource::LineToVerticalSegment>
    SVGPathByteStreamSource::ParseLineToVerticalSegment(PointF) noexcept
  {
    LineToVerticalSegment segment;
    segment.Y = ReadFloat();
    return segment;
  }

  Maybe<SVGPathSource::CurveToCubicSegment> SVGPathByteStreamSource::ParseCurveToCubicSegment(PointF) noexcept
  {
    CurveToCubicSegment segment;
    segment.Point1 = ReadPointF();
    segment.Point2 = ReadPointF();
    segment.TargetPoint = ReadPointF();
    return segment;
  }

  Maybe<SVGPathSource::CurveToCubicSmoothSegment>
    SVGPathByteStreamSource::ParseCurveToCubicSmoothSegment(PointF) noexcept
  {
    CurveToCubicSmoothSegment segment;
    segment.Point2 = ReadPointF();
    segment.TargetPoint = ReadPointF();
    return segment;
  }

  Maybe<SVGPathSource::CurveToQuadraticSegment>
    SVGPathByteStreamSource::ParseCurveToQuadraticSegment(PointF) noexcept
  {
    CurveToQuadraticSegment segment;
    segment.Point1 = ReadPointF();
    segment.TargetPoint = ReadPointF();
    return segment;
  }

  Maybe<SVGPathSource::CurveToQuadraticSmoothSegment>
    SVGPathByteStreamSource::ParseCurveToQuadraticSmoothSegment(PointF) noexcept
  {
    CurveToQuadraticSmoothSegment segment;
    segment.TargetPoint = ReadPointF();
    return segment;
  }

  Maybe<SVGPathSource::ArcToSegment> SVGPathByteStreamSource::ParseArcToSegment(PointF) noexcept
  {
    ArcToSegment segment;
    segment.Rx = ReadFloat();
    segment.Ry = ReadFloat();
    segment.Angle = ReadFloat();
    segment.LargeArc = ReadFlag();
    segment.Sweep = ReadFlag();
    segment.TargetPoint = ReadPointF();
    return segment;
  }
}

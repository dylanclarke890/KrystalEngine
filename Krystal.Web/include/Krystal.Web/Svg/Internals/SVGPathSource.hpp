/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Svg/Internals/SVGPathSegType.hpp"

namespace krys::web
{
  /// @brief Produces the segments of an SVG path, from a string or a byte stream.
  class SVGPathSource
  {
    KRYS_NON_COPYABLE(SVGPathSource);

  public:
    struct MoveToSegment
    {
      PointF TargetPoint;
    };

    struct LineToSegment
    {
      PointF TargetPoint;
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
      PointF Point1;
      PointF Point2;
      PointF TargetPoint;
    };

    struct CurveToCubicSmoothSegment
    {
      PointF Point2;
      PointF TargetPoint;
    };

    struct CurveToQuadraticSegment
    {
      PointF Point1;
      PointF TargetPoint;
    };

    struct CurveToQuadraticSmoothSegment
    {
      PointF TargetPoint;
    };

    struct ArcToSegment
    {
      float Rx {0};
      float Ry {0};
      float Angle {0};
      bool LargeArc {false};
      bool Sweep {false};
      PointF TargetPoint;
    };

    SVGPathSource() noexcept = default;

    virtual ~SVGPathSource() noexcept = default;

    KRYS_NODISCARD virtual bool HasMoreData() const noexcept = 0;

    virtual bool MoveToNextToken() noexcept = 0;

    KRYS_NODISCARD virtual SVGPathSegType NextCommand(SVGPathSegType previousCommand) noexcept = 0;

    KRYS_NODISCARD virtual Maybe<SVGPathSegType> ParseSVGSegmentType() noexcept = 0;

    KRYS_NODISCARD virtual Maybe<MoveToSegment> ParseMoveToSegment(PointF currentPoint) noexcept = 0;

    KRYS_NODISCARD virtual Maybe<LineToSegment> ParseLineToSegment(PointF currentPoint) noexcept = 0;

    KRYS_NODISCARD virtual Maybe<LineToHorizontalSegment>
      ParseLineToHorizontalSegment(PointF currentPoint) noexcept = 0;

    KRYS_NODISCARD virtual Maybe<LineToVerticalSegment>
      ParseLineToVerticalSegment(PointF currentPoint) noexcept = 0;

    KRYS_NODISCARD virtual Maybe<CurveToCubicSegment>
      ParseCurveToCubicSegment(PointF currentPoint) noexcept = 0;

    KRYS_NODISCARD virtual Maybe<CurveToCubicSmoothSegment>
      ParseCurveToCubicSmoothSegment(PointF currentPoint) noexcept = 0;

    KRYS_NODISCARD virtual Maybe<CurveToQuadraticSegment>
      ParseCurveToQuadraticSegment(PointF currentPoint) noexcept = 0;

    KRYS_NODISCARD virtual Maybe<CurveToQuadraticSmoothSegment>
      ParseCurveToQuadraticSmoothSegment(PointF currentPoint) noexcept = 0;

    KRYS_NODISCARD virtual Maybe<ArcToSegment> ParseArcToSegment(PointF currentPoint) noexcept = 0;
  };
}

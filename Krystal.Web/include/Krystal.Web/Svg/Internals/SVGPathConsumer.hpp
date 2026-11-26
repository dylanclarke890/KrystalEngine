/*
 * Copyright (C) 2002, 2003 The Karbon Developers
 * Copyright (C) 2006 Alexander Kellett <lypanov@kde.org>
 * Copyright (C) 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2007-2024 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::web::svg
{
  enum class PathCoordinateMode : uint8
  {
    AbsoluteCoordinates,
    RelativeCoordinates,
  };

  enum class PathParsingMode : uint8
  {
    // Every segment is converted to an absolute move, line, cubic curve or close.
    NormalisedParsing,
    // Segments are passed on as written.
    UnalteredParsing,
  };
}

namespace krys::web
{
  /// @brief Receives the segments of a parsed SVG path.
  class SVGPathConsumer
  {
    KRYS_NON_COPYABLE(SVGPathConsumer);

  public:
    SVGPathConsumer() noexcept = default;

    virtual void IncrementPathSegmentCount() noexcept = 0;

    KRYS_NODISCARD virtual bool ContinueConsuming() noexcept = 0;

    // Used in UnalteredParsing/NormalizedParsing modes.
    virtual void MoveTo(const PointF &targetPoint, bool closed, svg::PathCoordinateMode mode) noexcept = 0;

    virtual void LineTo(const PointF &targetPoint, svg::PathCoordinateMode mode) noexcept = 0;

    virtual void CurveToCubic(const PointF &controlPoint1, const PointF &controlPoint2,
                              const PointF &targetPoint, svg::PathCoordinateMode mode) noexcept = 0;

    virtual void ClosePath() noexcept = 0;

    // Only used in UnalteredParsing mode.
    virtual void LineToHorizontal(float x, svg::PathCoordinateMode mode) noexcept = 0;

    virtual void LineToVertical(float y, svg::PathCoordinateMode mode) noexcept = 0;

    virtual void CurveToCubicSmooth(const PointF &controlPoint2, const PointF &targetPoint,
                                    svg::PathCoordinateMode mode) noexcept = 0;

    virtual void CurveToQuadratic(const PointF &controlPoint, const PointF &targetPoint,
                                  svg::PathCoordinateMode mode) noexcept = 0;

    virtual void CurveToQuadraticSmooth(const PointF &targetPoint, svg::PathCoordinateMode mode) noexcept = 0;

    virtual void ArcTo(float r1, float r2, float angle, bool largeArcFlag, bool sweepFlag,
                       const PointF &targetPoint, svg::PathCoordinateMode mode) noexcept = 0;

  protected:
    virtual ~SVGPathConsumer() noexcept = default;
  };
}

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

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathConsumer.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::svg
{
  /// @brief Writes the segments it consumes as SVG path data, e.g. "M 0 0 L 10 10".
  class SVGPathStringBuilder final : public SVGPathConsumer
  {
  private:
    StringBuilder _stringBuilder;

  public:
    SVGPathStringBuilder() noexcept;
    ~SVGPathStringBuilder() noexcept final;

    KRYS_NODISCARD dom::DOMString Result() noexcept;

    void IncrementPathSegmentCount() noexcept final;
    KRYS_NODISCARD bool ContinueConsuming() noexcept final;

    // Used in UnalteredParsing/NormalizedParsing modes.
    void MoveTo(const FloatPoint &targetPoint, bool closed, PathCoordinateMode mode) noexcept final;
    void LineTo(const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept final;
    void CurveToCubic(const FloatPoint &point1, const FloatPoint &point2, const FloatPoint &targetPoint,
                      PathCoordinateMode mode) noexcept final;
    void ClosePath() noexcept final;

    // Only used in UnalteredParsing mode.
    void LineToHorizontal(float x, PathCoordinateMode mode) noexcept final;
    void LineToVertical(float y, PathCoordinateMode mode) noexcept final;
    void CurveToCubicSmooth(const FloatPoint &point2, const FloatPoint &targetPoint,
                            PathCoordinateMode mode) noexcept final;
    void CurveToQuadratic(const FloatPoint &point1, const FloatPoint &targetPoint,
                          PathCoordinateMode mode) noexcept final;
    void CurveToQuadraticSmooth(const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept final;
    void ArcTo(float r1, float r2, float angle, bool largeArcFlag, bool sweepFlag,
               const FloatPoint &targetPoint, PathCoordinateMode mode) noexcept final;
  };
}

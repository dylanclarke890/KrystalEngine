/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

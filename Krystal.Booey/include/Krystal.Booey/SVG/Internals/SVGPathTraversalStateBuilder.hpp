/*
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
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

#include "Krystal.Booey/Graphics/Geometry/FloatPoint.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathConsumer.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo
{
  class PathTraversalState;
}

namespace krys::boo::svg
{
  /// @brief Walks parsed SVG path data (in normalised parsing mode) with a @ref PathTraversalState, to
  /// measure it or find a point along it.
  class SVGPathTraversalStateBuilder final : public SVGPathConsumer
  {
  private:
    PathTraversalState &_traversalState;
    uint32 _segmentIndex {0};

  public:
    explicit SVGPathTraversalStateBuilder(PathTraversalState &, float desiredLength = 0) noexcept;

    KRYS_NODISCARD uint32 PathSegmentIndex() const noexcept
    {
      return _segmentIndex;
    }
    KRYS_NODISCARD float TotalLength() const noexcept;
    KRYS_NODISCARD FloatPoint CurrentPoint() const noexcept;

    void IncrementPathSegmentCount() noexcept final
    {
      ++_segmentIndex;
    }
    KRYS_NODISCARD bool ContinueConsuming() noexcept final;

  private:
    // Used in UnalteredParsing/NormalizedParsing modes.
    void MoveTo(const FloatPoint &, bool closed, PathCoordinateMode) noexcept final;
    void LineTo(const FloatPoint &, PathCoordinateMode) noexcept final;
    void CurveToCubic(const FloatPoint &, const FloatPoint &, const FloatPoint &,
                      PathCoordinateMode) noexcept final;
    void ClosePath() noexcept final;

    // Not used for PathTraversalState.
    void LineToHorizontal(float, PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }
    void LineToVertical(float, PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }
    void CurveToCubicSmooth(const FloatPoint &, const FloatPoint &, PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }
    void CurveToQuadratic(const FloatPoint &, const FloatPoint &, PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }
    void CurveToQuadraticSmooth(const FloatPoint &, PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }
    void ArcTo(float, float, float, bool, bool, const FloatPoint &, PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }
  };
}

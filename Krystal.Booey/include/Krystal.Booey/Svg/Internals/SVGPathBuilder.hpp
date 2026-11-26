/*
 * Copyright (C) 2002, 2003 The Karbon Developers
 * Copyright (C) 2006 Alexander Kellett <lypanov@kde.org>
 * Copyright (C) 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2007, 2009, 2015 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Svg/Internals/SVGPathConsumer.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Geometry/Point.hpp"

namespace krys::boo
{
  class Path;

  /// @brief Builds a `Path` from parsed SVG path data (in unaltered or normalised parsing mode).
  class SVGPathBuilder final : public SVGPathConsumer
  {
  private:
    Path &_path; // FIXME: This should use a WeakRef.
    PointF _current;

  public:
    explicit SVGPathBuilder(Path &) noexcept;

  private:
    void IncrementPathSegmentCount() noexcept final
    {
    }

    KRYS_NODISCARD bool ContinueConsuming() noexcept final
    {
      return true;
    }

    // Used in UnalteredParsing/NormalizedParsing modes.
    void MoveTo(const PointF &, bool closed, svg::PathCoordinateMode) noexcept final;

    void LineTo(const PointF &, svg::PathCoordinateMode) noexcept final;

    void CurveToCubic(const PointF &, const PointF &, const PointF &, svg::PathCoordinateMode) noexcept final;

    void ClosePath() noexcept final;

    // Only used in UnalteredParsing mode.
    void LineToHorizontal(float, svg::PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }

    void LineToVertical(float, svg::PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }

    void CurveToCubicSmooth(const PointF &, const PointF &, svg::PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }

    void CurveToQuadratic(const PointF &, const PointF &, svg::PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }

    void CurveToQuadraticSmooth(const PointF &, svg::PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }

    void ArcTo(float, float, float, bool, bool, const PointF &, svg::PathCoordinateMode) noexcept final
    {
      krys_debug_assert_msg(false, "Unreachable");
    }
  };
}

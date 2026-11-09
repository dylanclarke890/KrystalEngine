/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2015-2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Internals/SVGPathConsumer.hpp"

namespace krys::boo::svg
{
  enum class FloatBlendMode : bool
  {
    Horizontal,
    Vertical
  };

  class SVGPathSource;

  /// @brief Interpolates or adds two SVG paths segment by segment, or checks whether they can be.
  class SVGPathBlender
  {
    KRYS_NON_COPYABLE(SVGPathBlender);

  public:
    KRYS_NODISCARD static bool AddAnimatedPath(SVGPathSource &from, SVGPathSource &to, SVGPathConsumer &,
                                               unsigned repeatCount) noexcept;
    KRYS_NODISCARD static bool BlendAnimatedPath(SVGPathSource &from, SVGPathSource &to, SVGPathConsumer &,
                                                 float) noexcept;

    KRYS_NODISCARD static bool CanBlendPaths(SVGPathSource &from, SVGPathSource &to) noexcept;

  private:
    SVGPathBlender(SVGPathSource &, SVGPathSource &, SVGPathConsumer * = nullptr) noexcept;

    KRYS_NODISCARD bool CanBlendPaths() noexcept;

    KRYS_NODISCARD bool AddAnimatedPath(unsigned repeatCount) noexcept;
    KRYS_NODISCARD bool BlendAnimatedPath(float progress) noexcept;

    KRYS_NODISCARD bool BlendMoveToSegment(float progress) noexcept;
    KRYS_NODISCARD bool BlendLineToSegment(float progress) noexcept;
    KRYS_NODISCARD bool BlendLineToHorizontalSegment(float progress) noexcept;
    KRYS_NODISCARD bool BlendLineToVerticalSegment(float progress) noexcept;
    KRYS_NODISCARD bool BlendCurveToCubicSegment(float progress) noexcept;
    KRYS_NODISCARD bool BlendCurveToCubicSmoothSegment(float progress) noexcept;
    KRYS_NODISCARD bool BlendCurveToQuadraticSegment(float progress) noexcept;
    KRYS_NODISCARD bool BlendCurveToQuadraticSmoothSegment(float progress) noexcept;
    KRYS_NODISCARD bool BlendArcToSegment(float progress) noexcept;

    KRYS_NODISCARD float BlendAnimatedDimensonalFloat(float from, float to, FloatBlendMode,
                                                      float progress) noexcept;
    KRYS_NODISCARD FloatPoint BlendAnimatedFloatPoint(const FloatPoint &from, const FloatPoint &to,
                                                      float progress) noexcept;

    SVGPathSource &_fromSource;
    SVGPathSource &_toSource;
    SVGPathConsumer *_consumer; // A null consumer indicates that we're just checking blendability.

    FloatPoint _fromCurrentPoint;
    FloatPoint _toCurrentPoint;

    FloatPoint _fromSubpathPoint;
    FloatPoint _toSubpathPoint;

    PathCoordinateMode _fromMode {PathCoordinateMode::AbsoluteCoordinates};
    PathCoordinateMode _toMode {PathCoordinateMode::AbsoluteCoordinates};
    unsigned _addTypesCount {0};
    bool _isInFirstHalfOfAnimation {false};
  };
}

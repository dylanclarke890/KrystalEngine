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

#pragma once

#include "Krystal.Booey/SVG/Internals/SVGPathConsumer.hpp"

namespace krys::boo::svg
{
  // This consumer translates incoming SVG draw commands into absolute coordinates, and forwards it
  // to another consumer. convertSVGPathByteStreamToAbsoluteCoordinates uses it to convert relative
  // draw commands in an SVG path into absolute.
  /// @brief Forwards parsed SVG path data to another consumer with every segment in absolute coordinates.
  class SVGPathAbsoluteConverter final : public SVGPathConsumer
  {
  public:
    explicit SVGPathAbsoluteConverter(SVGPathConsumer &) noexcept;

  private:
    void IncrementPathSegmentCount() noexcept final;
    KRYS_NODISCARD bool ContinueConsuming() noexcept final;

    // Used in PathParsingMode::UnalteredParsing/PathParsingMode::NormalizedParsing modes.
    void MoveTo(const FloatPoint &targetPoint, bool closed, PathCoordinateMode) noexcept final;
    void LineTo(const FloatPoint &targetPoint, PathCoordinateMode) noexcept final;
    void CurveToCubic(const FloatPoint &point1, const FloatPoint &point2, const FloatPoint &targetPoint,
                      PathCoordinateMode) noexcept final;
    void ClosePath() noexcept final;

    // Only used in PathParsingMode::UnalteredParsing mode.
    void LineToHorizontal(float targetX, PathCoordinateMode) noexcept final;
    void LineToVertical(float targetY, PathCoordinateMode) noexcept final;
    void CurveToCubicSmooth(const FloatPoint &point2, const FloatPoint &targetPoint,
                            PathCoordinateMode) noexcept final;
    void CurveToQuadratic(const FloatPoint &point1, const FloatPoint &targetPoint,
                          PathCoordinateMode) noexcept final;
    void CurveToQuadraticSmooth(const FloatPoint &targetPoint, PathCoordinateMode) noexcept final;
    void ArcTo(float r1, float r2, float angle, bool largeArcFlag, bool sweepFlag,
               const FloatPoint &targetPoint, PathCoordinateMode) noexcept final;

    SVGPathConsumer &_consumer;
    FloatPoint _currentPoint;
    FloatPoint _subpathPoint;
  };
}

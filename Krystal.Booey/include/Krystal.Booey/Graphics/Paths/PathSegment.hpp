/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Paths/PathSegmentData.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <span>

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class PathSegment
  {
  public:
    using Data = Variant<PathMoveTo,

                         PathLineTo, PathQuadCurveTo, PathBezierCurveTo, PathArcTo,

                         PathArc, PathClosedArc, PathEllipse, PathEllipseInRect, PathRect, PathRoundedRect,
                         PathContinuousRoundedRect,

                         PathDataLine, PathDataQuadCurve, PathDataBezierCurve, PathDataArc,

                         PathCloseSubpath>;

    PathSegment(Data &&) noexcept;

    KRYS_NODISCARD bool operator==(const PathSegment &) const noexcept = default;

    KRYS_NODISCARD const Data &GetData() const & noexcept
    {
      return _data;
    }
    KRYS_NODISCARD Data &&GetData() && noexcept
    {
      return krys::move(_data);
    }
    KRYS_NODISCARD bool ClosesSubpath() const noexcept
    {
      return std::holds_alternative<PathCloseSubpath>(_data) || std::holds_alternative<PathClosedArc>(_data);
    }

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    KRYS_NODISCARD FloatRect FastBoundingRect() const noexcept;
    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;

    KRYS_NODISCARD bool CanApplyElements() const noexcept;
    KRYS_NODISCARD bool ApplyElements(const PathElementApplier &) const noexcept;

    KRYS_NODISCARD bool CanTransform() const noexcept;
    KRYS_NODISCARD bool Transform(const AffineTransform &) noexcept;

  private:
    Data _data;
  };

  using PathSegmentApplier = Func<void(const PathSegment &)>;

  inline PathSegment::PathSegment(Data &&data) noexcept : _data(krys::move(data))
  {
  }
}

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, const PathSegment &segment) noexcept;
}

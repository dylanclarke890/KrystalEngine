/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/RectEdges.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <algorithm>
#include <utility>

namespace krys::boo
{
  KRYS_NODISCARD float EuclidianDistance(const SizeF &delta) noexcept;

  KRYS_NODISCARD float EuclidianDistance(const PointF &p1, const PointF &p2) noexcept;

  // Find point where lines through the two pairs of points intersect. Returns false if the lines don't
  // intersect.
  KRYS_NODISCARD bool FindIntersection(const PointF &p1, const PointF &p2, const PointF &d1, const PointF &d2,
                                       PointF &intersection) noexcept;

  KRYS_NODISCARD RectI UnionRect(Span<const RectI> rects) noexcept;

  KRYS_NODISCARD RectI UnionRectIgnoringZeroRects(Span<const RectI> rects) noexcept;

  KRYS_NODISCARD RectF UnionRect(Span<const RectF> rects) noexcept;

  KRYS_NODISCARD RectF UnionRectIgnoringZeroRects(Span<const RectF> rects) noexcept;

  // Map point from srcRect to an equivalent point in destRect.
  KRYS_NODISCARD PointF MapPoint(PointF p, const RectF &srcRect, const RectF &destRect) noexcept;

  // Map rect from srcRect to an equivalent rect in destRect.
  KRYS_NODISCARD RectF MapRect(const RectF &r, const RectF &srcRect, const RectF &destRect) noexcept;

  KRYS_NODISCARD RectF LargestRectWithAspectRatioInsideRect(float aspectRatio, const RectF &srcRect) noexcept;

  KRYS_NODISCARD RectF SmallestRectWithAspectRatioAroundRect(float aspectRatio,
                                                             const RectF &srcRect) noexcept;

  KRYS_NODISCARD SizeF SizeWithAreaAndAspectRatio(float area, float aspectRatio) noexcept;

  // Compute a rect that encloses all points covered by the given rect if it were rotated a full turn around
  // (0,0).
  KRYS_NODISCARD RectF BoundsOfRotatingRect(const RectF &r) noexcept;

  KRYS_NODISCARD bool EllipseContainsPoint(const PointF &center, const SizeF &radii,
                                           const PointF &point) noexcept;

  KRYS_NODISCARD PointF MidPoint(const PointF &first, const PointF &second) noexcept;

  // -------------
  // |   h\  |s  |
  // |     \a|   |
  // |      \|   |
  // |       *   |
  // |     (x,y) |
  // -------------
  // Given a box and a ray (described by an offset from the top left corner of the box and angle from vertical
  // in degrees), compute the length from the starting position to the intersection of the ray with the box.
  // Given the above diagram, we are trying to calculate h, with lengthOfPointToSideOfIntersection computing
  // the length of s, and angleOfPointToSideOfIntersection computing a.
  KRYS_NODISCARD double LengthOfRayIntersectionWithBoundingBox(const RectF &boundingRect,
                                                               std::pair<const PointF &, float> ray) noexcept;

  // Given a box and a ray (described by an offset from the top left corner of the box and angle from vertical
  // in degrees), compute the closest length from the starting position to the side that the ray intersects
  // with.
  KRYS_NODISCARD double LengthOfPointToSideOfIntersection(const RectF &boundingRect,
                                                          std::pair<const PointF &, float> ray) noexcept;

  // Given a box and a ray (described by an offset from the top left corner of the box and angle from vertical
  // in degrees) compute the acute angle between the ray and the line segment from the starting point to the
  // closest point on the side that the ray intersects with.
  KRYS_NODISCARD float AngleOfPointToSideOfIntersection(const RectF &boundingRect,
                                                        std::pair<const PointF &, float> ray) noexcept;

  // Given a box and an offset from the top left corner, calculate the distance of the point from each side
  KRYS_NODISCARD RectEdges<double> DistanceOfPointToSidesOfRect(const RectF &box,
                                                                const PointF &position) noexcept;

  KRYS_NODISCARD float DistanceToClosestSide(PointF p, SizeF size) noexcept;

  KRYS_NODISCARD float DistanceToFarthestSide(PointF p, SizeF size) noexcept;

  KRYS_NODISCARD float DistanceToClosestCorner(PointF p, SizeF size) noexcept;

  KRYS_NODISCARD float DistanceToFarthestCorner(PointF p, SizeF size) noexcept;

  // Given a box and an offset from the top left corner, construct a coordinate system with this offset as the
  // origin, and return the vertices of the box in this coordinate system
  KRYS_NODISCARD Array<PointF, 4> VerticesForBox(const RectF &box, const PointF position) noexcept;

  KRYS_NODISCARD float ToPositiveAngle(float angle) noexcept;

  KRYS_NODISCARD float ToRelatedAcuteAngle(float angle) noexcept;

  KRYS_NODISCARD float NormaliseAngleInRadians(float radians) noexcept;

  KRYS_NODISCARD RectF ScaledRectAtOrigin(const RectF &rect, float scale, const PointF &origin) noexcept;

  struct RotatedRect
  {
    PointF Center;
    SizeF Size;
    float AngleInRadians;
  };

  KRYS_NODISCARD RotatedRect RotatedBoundingRectWithMinimumAngleOfRotation(
    const QuadF &quad, Maybe<float> minRotationInRadians = {}) noexcept;

  KRYS_NODISCARD constexpr float Min3(float a, float b, float c) noexcept
  {
    return std::min({a, b, c});
  }

  KRYS_NODISCARD constexpr float Max3(float a, float b, float c) noexcept
  {
    return std::max({a, b, c});
  }
}

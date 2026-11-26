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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Web/Graphics/Geometry/RectEdges.hpp"
#include <utility>

namespace krys::web
{
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
}

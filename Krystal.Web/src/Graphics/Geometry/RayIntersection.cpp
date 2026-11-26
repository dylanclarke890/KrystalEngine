/*
 * Copyright (C) 2014-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Google Inc. All rights reserved.
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

#include "Krystal.Web/Graphics/Geometry/RayIntersection.hpp"
#include "Krystal.Core/Geometry/GeometryUtilities.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Web/Graphics/Geometry/BoxSides.hpp"
#include <cmath>

namespace krys::web
{
  RectEdges<double> DistanceOfPointToSidesOfRect(const RectF &box, const PointF &position) noexcept
  {
    // Compute distance to each side of the containing box
    double top = std::abs(position.Y());
    double bottom = std::abs(position.Y() - box.Height());
    double left = std::abs(position.X());
    double right = std::abs(position.X() - box.Width());
    return RectEdges<double>(top, right, bottom, left);
  }

  double LengthOfRayIntersectionWithBoundingBox(const RectF &boundingRect,
                                                std::pair<const PointF &, float> ray) noexcept
  {
    auto length = LengthOfPointToSideOfIntersection(boundingRect, ray);
    auto angleOfTriangle = AngleOfPointToSideOfIntersection(boundingRect, ray);

    // Given a length and angle of a right triangle, calculate the hypotenuse, which corresponds to
    // the length from the given point to the intersecting point on the box
    return length / std::cos(deg2rad(angleOfTriangle));
  }

  namespace
  {
    // Get the side of box the ray intersects with
    KRYS_NODISCARD BoxSide IntersectionSide(const RectF &boundingRect,
                                            std::pair<const PointF &, float> ray) noexcept
    {
      auto position = ray.first;
      auto angleInRadians = deg2rad(ray.second);
      auto distances = DistanceOfPointToSidesOfRect(boundingRect, position);

      // Get possible intersection sides
      auto s1 = std::cos(angleInRadians) >= 0 ? distances.Top() : distances.Bottom();
      auto s2 = std::sin(angleInRadians) >= 0 ? distances.Right() : distances.Left();
      auto vertical = std::cos(angleInRadians) >= 0 ? BoxSide::Top : BoxSide::Bottom;
      auto horizontal = std::sin(angleInRadians) >= 0 ? BoxSide::Right : BoxSide::Left;
      auto acuteAngle = deg2rad(ToRelatedAcuteAngle(ray.second));

      return std::sin(acuteAngle) * s1 > std::cos(acuteAngle) * s2 ? horizontal : vertical;
    }
  }

  double LengthOfPointToSideOfIntersection(const RectF &boundingRect,
                                           std::pair<const PointF &, float> ray) noexcept
  {
    auto position = ray.first;
    if (position.X() < 0 || position.X() > boundingRect.Width() || position.Y() < 0
        || position.Y() > boundingRect.Height())
    {
      return 0;
    }
    auto distances = DistanceOfPointToSidesOfRect(boundingRect, position);
    return distances.At(IntersectionSide(boundingRect, ray));
  }

  float AngleOfPointToSideOfIntersection(const RectF &boundingRect,
                                         std::pair<const PointF &, float> ray) noexcept
  {
    auto angle = ray.second;
    auto side = IntersectionSide(boundingRect, ray);
    angle = ToRelatedAcuteAngle(angle);
    return side == BoxSide::Top || side == BoxSide::Bottom ? angle : 90 - angle;
  }
}

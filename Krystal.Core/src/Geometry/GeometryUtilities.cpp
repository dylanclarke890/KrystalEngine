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

#include "Krystal.Core/Geometry/GeometryUtilities.hpp"
#include "Krystal.Core/Geometry/Quad.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include <cmath>
#include <numbers>
#include <numeric>

namespace krys
{
  float EuclidianDistance(const SizeF &delta) noexcept
  {
    return std::hypot(delta.Width(), delta.Height());
  }

  float EuclidianDistance(const PointF &p1, const PointF &p2) noexcept
  {
    return EuclidianDistance(p1 - p2);
  }

  bool FindIntersection(const PointF &p1, const PointF &p2, const PointF &d1, const PointF &d2,
                        PointF &intersection) noexcept
  {
    float pxLength = p2.X() - p1.X();
    float pyLength = p2.Y() - p1.Y();

    float dxLength = d2.X() - d1.X();
    float dyLength = d2.Y() - d1.Y();

    float denom = pxLength * dyLength - pyLength * dxLength;
    if (!denom)
    {
      return false;
    }

    float param = ((d1.X() - p1.X()) * dyLength - (d1.Y() - p1.Y()) * dxLength) / denom;

    intersection.SetX(p1.X() + param * pxLength);
    intersection.SetY(p1.Y() + param * pyLength);
    return true;
  }

  RectI UnionRect(Span<const RectI> rects) noexcept
  {
    RectI result;
    for (auto &rect : rects)
    {
      result.Unite(rect);
    }

    return result;
  }

  RectI UnionRectIgnoringZeroRects(Span<const RectI> rects) noexcept
  {
    RectI result;
    for (auto &rect : rects)
    {
      result.UniteIfNonZero(rect);
    }

    return result;
  }

  RectF UnionRect(Span<const RectF> rects) noexcept
  {
    RectF result;
    for (auto &rect : rects)
    {
      result.Unite(rect);
    }

    return result;
  }

  RectF UnionRectIgnoringZeroRects(Span<const RectF> rects) noexcept
  {
    RectF result;
    for (auto &rect : rects)
    {
      result.UniteIfNonZero(rect);
    }

    return result;
  }

  PointF MapPoint(PointF p, const RectF &srcRect, const RectF &destRect) noexcept
  {
    if (!srcRect.Width() || !srcRect.Height())
    {
      return p;
    }

    float widthScale = destRect.Width() / srcRect.Width();
    float heightScale = destRect.Height() / srcRect.Height();
    return {destRect.X() + (p.X() - srcRect.X()) * widthScale,
            destRect.Y() + (p.Y() - srcRect.Y()) * heightScale};
  }

  RectF MapRect(const RectF &r, const RectF &srcRect, const RectF &destRect) noexcept
  {
    if (!srcRect.Width() || !srcRect.Height())
    {
      return RectF();
    }

    float widthScale = destRect.Width() / srcRect.Width();
    float heightScale = destRect.Height() / srcRect.Height();
    return {destRect.X() + (r.X() - srcRect.X()) * widthScale,
            destRect.Y() + (r.Y() - srcRect.Y()) * heightScale, r.Width() * widthScale,
            r.Height() * heightScale};
  }

  RectF LargestRectWithAspectRatioInsideRect(float aspectRatio, const RectF &srcRect) noexcept
  {
    RectF destRect = srcRect;

    if (aspectRatio > srcRect.Size().AspectRatio())
    {
      float dy = destRect.Width() / aspectRatio - destRect.Height();
      destRect.InflateY(dy / 2);
    }
    else
    {
      float dx = destRect.Height() * aspectRatio - destRect.Width();
      destRect.InflateX(dx / 2);
    }
    return destRect;
  }

  RectF BoundsOfRotatingRect(const RectF &r) noexcept
  {
    // Compute the furthest corner from the origin.
    float maxCornerDistance = EuclidianDistance(PointF(), r.MinXMinYCorner());
    maxCornerDistance = std::max(maxCornerDistance, EuclidianDistance(PointF(), r.MaxXMinYCorner()));
    maxCornerDistance = std::max(maxCornerDistance, EuclidianDistance(PointF(), r.MinXMaxYCorner()));
    maxCornerDistance = std::max(maxCornerDistance, EuclidianDistance(PointF(), r.MaxXMaxYCorner()));

    return RectF(-maxCornerDistance, -maxCornerDistance, 2 * maxCornerDistance, 2 * maxCornerDistance);
  }

  RectF SmallestRectWithAspectRatioAroundRect(float aspectRatio, const RectF &srcRect) noexcept
  {
    RectF destRect = srcRect;

    if (aspectRatio < srcRect.Size().AspectRatio())
    {
      float dy = destRect.Width() / aspectRatio - destRect.Height();
      destRect.InflateY(dy / 2);
    }
    else
    {
      float dx = destRect.Height() * aspectRatio - destRect.Width();
      destRect.InflateX(dx / 2);
    }
    return destRect;
  }

  SizeF SizeWithAreaAndAspectRatio(float area, float aspectRatio) noexcept
  {
    auto scaledWidth = std::sqrt(area * aspectRatio);
    return {scaledWidth, scaledWidth / aspectRatio};
  }

  bool EllipseContainsPoint(const PointF &center, const SizeF &radii, const PointF &point) noexcept
  {
    if (radii.Width() <= 0 || radii.Height() <= 0)
    {
      return false;
    }

    // First, offset the query point so that the ellipse is effectively centered at the origin.
    PointF transformedPoint(point);
    transformedPoint.Move(-center.X(), -center.Y());

    // If the point lies outside of the bounding box determined by the radii of the ellipse, it can't possibly
    // be contained within the ellipse, so bail early.
    if (transformedPoint.X() < -radii.Width() || transformedPoint.X() > radii.Width()
        || transformedPoint.Y() < -radii.Height() || transformedPoint.Y() > radii.Height())
    {
      return false;
    }

    // Let (x, y) represent the translated point, and let (Rx, Ry) represent the radii of an ellipse centered
    // at the origin. (x, y) is contained within the ellipse if, after scaling the ellipse to be a unit
    // circle, the identically scaled point lies within that unit circle. In other words, the squared distance
    // (x/Rx)^2 + (y/Ry)^2 of the transformed point to the origin is no greater than 1. This is equivalent to
    // checking whether or not the point (xRy, yRx) lies within a circle of radius RxRy.
    transformedPoint.Scale(radii.Height(), radii.Width());
    auto transformedRadius = radii.Width() * radii.Height();

    // We can bail early if |xRy| + |yRx| <= RxRy to avoid additional multiplications, since that means the
    // Manhattan distance of the transformed point is less than the radius, so the point must lie within the
    // transformed circle.
    return std::abs(transformedPoint.X()) + std::abs(transformedPoint.Y()) <= transformedRadius
           || transformedPoint.LengthSquared() <= transformedRadius * transformedRadius;
  }

  PointF MidPoint(const PointF &first, const PointF &second) noexcept
  {
    return {std::midpoint(first.X(), second.X()), std::midpoint(first.Y(), second.Y())};
  }

  namespace
  {
    KRYS_NODISCARD float DotProduct(const SizeF &u, const SizeF &v) noexcept
    {
      return u.Width() * v.Width() + u.Height() * v.Height();
    }

    KRYS_NODISCARD float AngleBetweenVectors(const SizeF &u, const SizeF &v) noexcept
    {
      auto magnitudes = u.DiagonalLength() * v.DiagonalLength();
      return magnitudes ? std::acos(ClampTo<float>(DotProduct(u, v) / magnitudes, -1, 1)) : 0;
    }
  }

  RotatedRect RotatedBoundingRectWithMinimumAngleOfRotation(const QuadF &quad,
                                                            Maybe<float> minRotationInRadians) noexcept
  {
    constexpr auto twoPiFloat = 2 * std::numbers::pi_v<float>;

    auto minRotationAmount = minRotationInRadians.value_or(Limits<float>::epsilon());
    auto leftMidPoint = MidPoint(quad.P1(), quad.P4());
    auto rightMidPoint = MidPoint(quad.P2(), quad.P3());
    auto widthVector = rightMidPoint - leftMidPoint;
    auto midPointToMidPointDistance = widthVector.DiagonalLength();

    int signOfWidthVectorHeight = widthVector.Height() < 0 ? -1 : 1;
    auto angle = midPointToMidPointDistance
                   ? signOfWidthVectorHeight * std::acos(widthVector.Width() / midPointToMidPointDistance)
                   : 0;
    if (angle < 0)
    {
      angle += twoPiFloat;
    }

    if (std::abs(angle) < minRotationAmount || std::abs(twoPiFloat - angle) < minRotationAmount)
    {
      auto boundingBox = quad.BoundingBox();
      return {boundingBox.Center(), boundingBox.Size(), 0};
    }

    auto heightVector = SizeF {widthVector.Height(), -widthVector.Width()};
    auto leftPerpendicularAngle = AngleBetweenVectors(heightVector, quad.P1() - leftMidPoint);
    auto rightPerpendicularAngle = AngleBetweenVectors(heightVector, quad.P2() - rightMidPoint);

    auto leftHypotenuseLength = (leftMidPoint - quad.P1()).DiagonalLength();
    auto rightHypotenuseLength = (rightMidPoint - quad.P2()).DiagonalLength();

    auto leftMargin = leftHypotenuseLength * std::sin(leftPerpendicularAngle);
    auto rightMargin = rightHypotenuseLength * std::sin(rightPerpendicularAngle);

    auto width = midPointToMidPointDistance + leftMargin + rightMargin;
    auto height = 2
                  * std::max(leftHypotenuseLength * std::cos(leftPerpendicularAngle),
                             rightHypotenuseLength * std::cos(rightPerpendicularAngle));

    auto leftMidToCenterDistance = (midPointToMidPointDistance + rightMargin - leftMargin) / 2;
    auto center = leftMidPoint + (widthVector * leftMidToCenterDistance / midPointToMidPointDistance);

    return {center, {width, height}, angle};
  }

  float ToPositiveAngle(float angle) noexcept
  {
    angle = std::fmod(angle, 360.0f);
    while (angle < 0)
    {
      angle += 360.0f;
    }
    return angle;
  }

  // Compute acute angle from vertical axis
  float ToRelatedAcuteAngle(float angle) noexcept
  {
    angle = ToPositiveAngle(angle);
    if (angle < 90)
    {
      return angle;
    }

    // FIXME: webkit.org/b/298890 toRelatedAcuteAngle in GeometryUtilities.cpp doesn't handle an angle greater
    // than 270 degrees
    return std::abs(180 - angle);
  }

  float DistanceToClosestSide(PointF p, SizeF size) noexcept
  {
    float widthDelta = std::abs(size.Width() - p.X());
    float heightDelta = std::abs(size.Height() - p.Y());
    return Min4(std::abs(p.X()), widthDelta, std::abs(p.Y()), heightDelta);
  }

  float DistanceToFarthestSide(PointF p, SizeF size) noexcept
  {
    float widthDelta = std::abs(size.Width() - p.X());
    float heightDelta = std::abs(size.Height() - p.Y());
    return Max4(std::abs(p.X()), widthDelta, std::abs(p.Y()), heightDelta);
  }

  float DistanceToClosestCorner(PointF p, SizeF size) noexcept
  {
    PointF topLeft;
    float topLeftDistance = SizeF(p - topLeft).DiagonalLength();
    PointF topRight(size.Width(), 0);
    float topRightDistance = SizeF(p - topRight).DiagonalLength();
    PointF bottomLeft(0, size.Height());
    float bottomLeftDistance = SizeF(p - bottomLeft).DiagonalLength();
    PointF bottomRight(size.Width(), size.Height());
    float bottomRightDistance = SizeF(p - bottomRight).DiagonalLength();
    return Min4(topLeftDistance, topRightDistance, bottomLeftDistance, bottomRightDistance);
  }

  float DistanceToFarthestCorner(PointF p, SizeF size) noexcept
  {
    PointF topLeft;
    float topLeftDistance = SizeF(p - topLeft).DiagonalLength();
    PointF topRight(size.Width(), 0);
    float topRightDistance = SizeF(p - topRight).DiagonalLength();
    PointF bottomLeft(0, size.Height());
    float bottomLeftDistance = SizeF(p - bottomLeft).DiagonalLength();
    PointF bottomRight(size.Width(), size.Height());
    float bottomRightDistance = SizeF(p - bottomRight).DiagonalLength();
    return Max4(topLeftDistance, topRightDistance, bottomLeftDistance, bottomRightDistance);
  }

  Array<PointF, 4> VerticesForBox(const RectF &box, const PointF position) noexcept
  {
    return {PointF(-position.X(), -position.Y()), PointF(box.Width() - position.X(), -position.Y()),
            PointF(box.Width() - position.X(), box.Height() - position.Y()),
            PointF(-position.X(), box.Height() - position.Y())};
  }

  float NormaliseAngleInRadians(float radians) noexcept
  {
    float circles = radians / radiansPerTurnFloat;
    return radiansPerTurnFloat * (circles - std::floor(circles));
  }

  RectF ScaledRectAtOrigin(const RectF &rect, float scale, const PointF &origin) noexcept
  {
    return {origin + (rect.Location() - origin) * scale, rect.Size() * scale};
  }
}

#pragma once

#include "Krystal.Booey/Core/RectEdges.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <algorithm>
#include <utility>

namespace krys::boo
{
  class FloatQuad;

  KRYS_NODISCARD float EuclidianDistance(const FloatSize &delta) noexcept;
  KRYS_NODISCARD float EuclidianDistance(const FloatPoint &p1, const FloatPoint &p2) noexcept;

  // Find point where lines through the two pairs of points intersect. Returns false if the lines don't
  // intersect.
  KRYS_NODISCARD bool FindIntersection(const FloatPoint &p1, const FloatPoint &p2, const FloatPoint &d1,
                                       const FloatPoint &d2, FloatPoint &intersection) noexcept;

  KRYS_NODISCARD IntRect UnionRect(Span<const IntRect> rects) noexcept;
  KRYS_NODISCARD IntRect UnionRectIgnoringZeroRects(Span<const IntRect> rects) noexcept;
  KRYS_NODISCARD FloatRect UnionRect(Span<const FloatRect> rects) noexcept;
  KRYS_NODISCARD FloatRect UnionRectIgnoringZeroRects(Span<const FloatRect> rects) noexcept;

  // Map point from srcRect to an equivalent point in destRect.
  KRYS_NODISCARD FloatPoint MapPoint(FloatPoint p, const FloatRect &srcRect,
                                     const FloatRect &destRect) noexcept;

  // Map rect from srcRect to an equivalent rect in destRect.
  KRYS_NODISCARD FloatRect MapRect(const FloatRect &r, const FloatRect &srcRect,
                                   const FloatRect &destRect) noexcept;

  KRYS_NODISCARD FloatRect LargestRectWithAspectRatioInsideRect(float aspectRatio,
                                                                const FloatRect &srcRect) noexcept;
  KRYS_NODISCARD FloatRect SmallestRectWithAspectRatioAroundRect(float aspectRatio,
                                                                 const FloatRect &srcRect) noexcept;

  KRYS_NODISCARD FloatSize SizeWithAreaAndAspectRatio(float area, float aspectRatio) noexcept;

  // Compute a rect that encloses all points covered by the given rect if it were rotated a full turn around
  // (0,0).
  KRYS_NODISCARD FloatRect BoundsOfRotatingRect(const FloatRect &r) noexcept;

  KRYS_NODISCARD bool EllipseContainsPoint(const FloatPoint &center, const FloatSize &radii,
                                           const FloatPoint &point) noexcept;

  KRYS_NODISCARD FloatPoint MidPoint(const FloatPoint &first, const FloatPoint &second) noexcept;

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
  KRYS_NODISCARD double
    LengthOfRayIntersectionWithBoundingBox(const FloatRect &boundingRect,
                                           std::pair<const FloatPoint &, float> ray) noexcept;

  // Given a box and a ray (described by an offset from the top left corner of the box and angle from vertical
  // in degrees), compute the closest length from the starting position to the side that the ray intersects
  // with.
  KRYS_NODISCARD double LengthOfPointToSideOfIntersection(const FloatRect &boundingRect,
                                                          std::pair<const FloatPoint &, float> ray) noexcept;

  // Given a box and a ray (described by an offset from the top left corner of the box and angle from vertical
  // in degrees) compute the acute angle between the ray and the line segment from the starting point to the
  // closest point on the side that the ray intersects with.
  KRYS_NODISCARD float AngleOfPointToSideOfIntersection(const FloatRect &boundingRect,
                                                        std::pair<const FloatPoint &, float> ray) noexcept;

  // Given a box and an offset from the top left corner, calculate the distance of the point from each side
  KRYS_NODISCARD RectEdges<double> DistanceOfPointToSidesOfRect(const FloatRect &box,
                                                                const FloatPoint &position) noexcept;

  KRYS_NODISCARD float DistanceToClosestSide(FloatPoint p, FloatSize size) noexcept;
  KRYS_NODISCARD float DistanceToFarthestSide(FloatPoint p, FloatSize size) noexcept;
  KRYS_NODISCARD float DistanceToClosestCorner(FloatPoint p, FloatSize size) noexcept;
  KRYS_NODISCARD float DistanceToFarthestCorner(FloatPoint p, FloatSize size) noexcept;

  // Given a box and an offset from the top left corner, construct a coordinate system with this offset as the
  // origin, and return the vertices of the box in this coordinate system
  KRYS_NODISCARD Array<FloatPoint, 4> VerticesForBox(const FloatRect &box,
                                                     const FloatPoint position) noexcept;

  KRYS_NODISCARD float ToPositiveAngle(float angle) noexcept;
  KRYS_NODISCARD float ToRelatedAcuteAngle(float angle) noexcept;
  KRYS_NODISCARD float NormaliseAngleInRadians(float radians) noexcept;

  KRYS_NODISCARD FloatRect ScaledRectAtOrigin(const FloatRect &rect, float scale,
                                              const FloatPoint &origin) noexcept;

  struct RotatedRect
  {
    FloatPoint Center;
    FloatSize Size;
    float AngleInRadians;
  };

  KRYS_NODISCARD RotatedRect RotatedBoundingRectWithMinimumAngleOfRotation(
    const FloatQuad &quad, Maybe<float> minRotationInRadians = {}) noexcept;

  KRYS_NODISCARD constexpr float Min3(float a, float b, float c) noexcept
  {
    return std::min(std::min(a, b), c);
  }

  KRYS_NODISCARD constexpr float Max3(float a, float b, float c) noexcept
  {
    return std::max(std::max(a, b), c);
  }

  KRYS_NODISCARD constexpr float Min4(float a, float b, float c, float d) noexcept
  {
    return std::min(std::min(a, b), std::min(c, d));
  }

  KRYS_NODISCARD constexpr float Max4(float a, float b, float c, float d) noexcept
  {
    return std::max(std::max(a, b), std::max(c, d));
  }
}

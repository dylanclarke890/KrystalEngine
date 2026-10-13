#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <cmath>
#include <numbers>
#include <numeric>

namespace krys::boo
{
  float EuclidianDistance(const FloatSize &delta) noexcept
  {
    return std::hypot(delta.Width(), delta.Height());
  }

  float EuclidianDistance(const FloatPoint &p1, const FloatPoint &p2) noexcept
  {
    return EuclidianDistance(p1 - p2);
  }

  bool FindIntersection(const FloatPoint &p1, const FloatPoint &p2, const FloatPoint &d1,
                        const FloatPoint &d2, FloatPoint &intersection) noexcept
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

  IntRect UnionRect(Span<const IntRect> rects) noexcept
  {
    IntRect result;
    for (auto &rect : rects)
    {
      result.Unite(rect);
    }

    return result;
  }

  IntRect UnionRectIgnoringZeroRects(Span<const IntRect> rects) noexcept
  {
    IntRect result;
    for (auto &rect : rects)
    {
      result.UniteIfNonZero(rect);
    }

    return result;
  }

  FloatRect UnionRect(Span<const FloatRect> rects) noexcept
  {
    FloatRect result;
    for (auto &rect : rects)
    {
      result.Unite(rect);
    }

    return result;
  }

  FloatRect UnionRectIgnoringZeroRects(Span<const FloatRect> rects) noexcept
  {
    FloatRect result;
    for (auto &rect : rects)
    {
      result.UniteIfNonZero(rect);
    }

    return result;
  }

  FloatPoint MapPoint(FloatPoint p, const FloatRect &srcRect, const FloatRect &destRect) noexcept
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

  FloatRect MapRect(const FloatRect &r, const FloatRect &srcRect, const FloatRect &destRect) noexcept
  {
    if (!srcRect.Width() || !srcRect.Height())
    {
      return FloatRect();
    }

    float widthScale = destRect.Width() / srcRect.Width();
    float heightScale = destRect.Height() / srcRect.Height();
    return {destRect.X() + (r.X() - srcRect.X()) * widthScale,
            destRect.Y() + (r.Y() - srcRect.Y()) * heightScale, r.Width() * widthScale,
            r.Height() * heightScale};
  }

  FloatRect LargestRectWithAspectRatioInsideRect(float aspectRatio, const FloatRect &srcRect) noexcept
  {
    FloatRect destRect = srcRect;

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

  FloatRect BoundsOfRotatingRect(const FloatRect &r) noexcept
  {
    // Compute the furthest corner from the origin.
    float maxCornerDistance = EuclidianDistance(FloatPoint(), r.MinXMinYCorner());
    maxCornerDistance = std::max(maxCornerDistance, EuclidianDistance(FloatPoint(), r.MaxXMinYCorner()));
    maxCornerDistance = std::max(maxCornerDistance, EuclidianDistance(FloatPoint(), r.MinXMaxYCorner()));
    maxCornerDistance = std::max(maxCornerDistance, EuclidianDistance(FloatPoint(), r.MaxXMaxYCorner()));

    return FloatRect(-maxCornerDistance, -maxCornerDistance, 2 * maxCornerDistance, 2 * maxCornerDistance);
  }

  FloatRect SmallestRectWithAspectRatioAroundRect(float aspectRatio, const FloatRect &srcRect) noexcept
  {
    FloatRect destRect = srcRect;

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

  FloatSize SizeWithAreaAndAspectRatio(float area, float aspectRatio) noexcept
  {
    auto scaledWidth = std::sqrt(area * aspectRatio);
    return {scaledWidth, scaledWidth / aspectRatio};
  }

  bool EllipseContainsPoint(const FloatPoint &center, const FloatSize &radii,
                            const FloatPoint &point) noexcept
  {
    if (radii.Width() <= 0 || radii.Height() <= 0)
    {
      return false;
    }

    // First, offset the query point so that the ellipse is effectively centered at the origin.
    FloatPoint transformedPoint(point);
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

  FloatPoint MidPoint(const FloatPoint &first, const FloatPoint &second) noexcept
  {
    return {std::midpoint(first.X(), second.X()), std::midpoint(first.Y(), second.Y())};
  }

  namespace
  {
    KRYS_NODISCARD float DotProduct(const FloatSize &u, const FloatSize &v) noexcept
    {
      return u.Width() * v.Width() + u.Height() * v.Height();
    }

    KRYS_NODISCARD float AngleBetweenVectors(const FloatSize &u, const FloatSize &v) noexcept
    {
      auto magnitudes = u.DiagonalLength() * v.DiagonalLength();
      return magnitudes ? std::acos(ClampTo<float>(DotProduct(u, v) / magnitudes, -1, 1)) : 0;
    }
  }

  RotatedRect RotatedBoundingRectWithMinimumAngleOfRotation(const FloatQuad &quad,
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

    auto heightVector = FloatSize {widthVector.Height(), -widthVector.Width()};
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

  RectEdges<double> DistanceOfPointToSidesOfRect(const FloatRect &box, const FloatPoint &position) noexcept
  {
    // Compute distance to each side of the containing box
    double top = std::abs(position.Y());
    double bottom = std::abs(position.Y() - box.Height());
    double left = std::abs(position.X());
    double right = std::abs(position.X() - box.Width());
    return RectEdges<double>(top, right, bottom, left);
  }

  float DistanceToClosestSide(FloatPoint p, FloatSize size) noexcept
  {
    float widthDelta = std::abs(size.Width() - p.X());
    float heightDelta = std::abs(size.Height() - p.Y());
    return Min4(std::abs(p.X()), widthDelta, std::abs(p.Y()), heightDelta);
  }

  float DistanceToFarthestSide(FloatPoint p, FloatSize size) noexcept
  {
    float widthDelta = std::abs(size.Width() - p.X());
    float heightDelta = std::abs(size.Height() - p.Y());
    return Max4(std::abs(p.X()), widthDelta, std::abs(p.Y()), heightDelta);
  }

  float DistanceToClosestCorner(FloatPoint p, FloatSize size) noexcept
  {
    FloatPoint topLeft;
    float topLeftDistance = FloatSize(p - topLeft).DiagonalLength();
    FloatPoint topRight(size.Width(), 0);
    float topRightDistance = FloatSize(p - topRight).DiagonalLength();
    FloatPoint bottomLeft(0, size.Height());
    float bottomLeftDistance = FloatSize(p - bottomLeft).DiagonalLength();
    FloatPoint bottomRight(size.Width(), size.Height());
    float bottomRightDistance = FloatSize(p - bottomRight).DiagonalLength();
    return Min4(topLeftDistance, topRightDistance, bottomLeftDistance, bottomRightDistance);
  }

  float DistanceToFarthestCorner(FloatPoint p, FloatSize size) noexcept
  {
    FloatPoint topLeft;
    float topLeftDistance = FloatSize(p - topLeft).DiagonalLength();
    FloatPoint topRight(size.Width(), 0);
    float topRightDistance = FloatSize(p - topRight).DiagonalLength();
    FloatPoint bottomLeft(0, size.Height());
    float bottomLeftDistance = FloatSize(p - bottomLeft).DiagonalLength();
    FloatPoint bottomRight(size.Width(), size.Height());
    float bottomRightDistance = FloatSize(p - bottomRight).DiagonalLength();
    return Max4(topLeftDistance, topRightDistance, bottomLeftDistance, bottomRightDistance);
  }

  Array<FloatPoint, 4> VerticesForBox(const FloatRect &box, const FloatPoint position) noexcept
  {
    return {FloatPoint(-position.X(), -position.Y()), FloatPoint(box.Width() - position.X(), -position.Y()),
            FloatPoint(box.Width() - position.X(), box.Height() - position.Y()),
            FloatPoint(-position.X(), box.Height() - position.Y())};
  }

  double LengthOfRayIntersectionWithBoundingBox(const FloatRect &boundingRect,
                                                std::pair<const FloatPoint &, float> ray) noexcept
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
    KRYS_NODISCARD BoxSide IntersectionSide(const FloatRect &boundingRect,
                                            std::pair<const FloatPoint &, float> ray) noexcept
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

  double LengthOfPointToSideOfIntersection(const FloatRect &boundingRect,
                                           std::pair<const FloatPoint &, float> ray) noexcept
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

  float AngleOfPointToSideOfIntersection(const FloatRect &boundingRect,
                                         std::pair<const FloatPoint &, float> ray) noexcept
  {
    auto angle = ray.second;
    auto side = IntersectionSide(boundingRect, ray);
    angle = ToRelatedAcuteAngle(angle);
    return side == BoxSide::Top || side == BoxSide::Bottom ? angle : 90 - angle;
  }

  float NormaliseAngleInRadians(float radians) noexcept
  {
    float circles = radians / radiansPerTurnFloat;
    return radiansPerTurnFloat * (circles - std::floor(circles));
  }

  FloatRect ScaledRectAtOrigin(const FloatRect &rect, float scale, const FloatPoint &origin) noexcept
  {
    return {origin + (rect.Location() - origin) * scale, rect.Size() * scale};
  }
}

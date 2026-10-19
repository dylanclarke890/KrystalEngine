#include "Krystal.Booey/Graphics/PathSegmentData.hpp"

#include "Krystal.Booey/Core/MathExtras.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <cmath>
#include <numbers>

namespace krys::boo
{
  inline void ExtendRect(FloatRect &rect, FloatPoint a, FloatPoint b, FloatPoint c) noexcept
  {
    auto minX = Min3(a.X(), b.X(), c.X());
    auto maxX = Max3(a.X(), b.X(), c.X());
    auto minY = Min3(a.Y(), b.Y(), c.Y());
    auto maxY = Max3(a.Y(), b.Y(), c.Y());

    rect.Extend({minX, minY}, {maxX, maxY});
  }

  inline void ExtendRect(FloatRect &rect, FloatPoint a, FloatPoint b, FloatPoint c, FloatPoint d) noexcept
  {
    auto minX = Min4(a.X(), b.X(), c.X(), d.X());
    auto maxX = Max4(a.X(), b.X(), c.X(), d.X());
    auto minY = Min4(a.Y(), b.Y(), c.Y(), d.Y());
    auto maxY = Max4(a.Y(), b.Y(), c.Y(), d.Y());

    rect.Extend({minX, minY}, {maxX, maxY});
  }

  FloatPoint PathMoveTo::CalculateEndPoint(const FloatPoint &, FloatPoint &lastMoveToPoint) const noexcept
  {
    lastMoveToPoint = Point;
    return lastMoveToPoint;
  }

  Maybe<FloatPoint> PathMoveTo::TryGetEndPointWithoutContext() const noexcept
  {
    FloatPoint lastMoveToPoint;
    return CalculateEndPoint({}, lastMoveToPoint);
  }

  void PathMoveTo::ExtendFastBoundingRect(const FloatPoint &, const FloatPoint &, FloatRect &) const noexcept
  {
  }

  void PathMoveTo::ExtendBoundingRect(const FloatPoint &, const FloatPoint &, FloatRect &) const noexcept
  {
  }

  void PathMoveTo::ApplyElements(const PathElementApplier &applier) const noexcept
  {
    applier({PathElement::Type::MoveToPoint, {Point}});
  }

  void PathMoveTo::Transform(const AffineTransform &transform) noexcept
  {
    Point = transform.MapPoint(Point);
  }

  FloatPoint PathLineTo::CalculateEndPoint(const FloatPoint &, FloatPoint &) const noexcept
  {
    return Point;
  }

  Maybe<FloatPoint> PathLineTo::TryGetEndPointWithoutContext() const noexcept
  {
    FloatPoint lastMoveToPoint;
    return CalculateEndPoint({}, lastMoveToPoint);
  }

  void PathLineTo::ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                          FloatRect &boundingRect) const noexcept
  {
    ExtendBoundingRect(currentPoint, lastMoveToPoint, boundingRect);
  }

  void PathLineTo::ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &,
                                      FloatRect &boundingRect) const noexcept
  {
    boundingRect.Extend(currentPoint);
    boundingRect.Extend(Point);
  }

  void PathLineTo::ApplyElements(const PathElementApplier &applier) const noexcept
  {
    applier({PathElement::Type::AddLineToPoint, {Point}});
  }

  void PathLineTo::Transform(const AffineTransform &transform) noexcept
  {
    Point = transform.MapPoint(Point);
  }

  FloatPoint PathQuadCurveTo::CalculateEndPoint(const FloatPoint &, FloatPoint &) const noexcept
  {
    return EndPoint;
  }

  Maybe<FloatPoint> PathQuadCurveTo::TryGetEndPointWithoutContext() const noexcept
  {
    FloatPoint lastMoveToPoint;
    return CalculateEndPoint({}, lastMoveToPoint);
  }

  void PathQuadCurveTo::ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &,
                                               FloatRect &boundingRect) const noexcept
  {
    ExtendRect(boundingRect, currentPoint, ControlPoint, EndPoint);
  }

  KRYS_NODISCARD static float CalculateQuadratic(float t, float p0, float p1, float p2) noexcept
  {
    // Fallback to a value which won't affect the calculations.
    if (!(t >= 0 && t <= 1))
    {
      return p0;
    }

    float s = 1 - t;

    // B(t) = (1 - t)^2 P0 + 2 (1 - t)t P1 + t^2 P2, 0 <= t <= 1
    return (s * s * p0) + (2 * s * t * p1) + (t * t * p2);
  }

  KRYS_NODISCARD static float CalculateQuadraticExtremity(float p0, float p1, float p2) noexcept
  {
    // B(t)  = (1 - t)^2 P0 + 2 (1 - t)t P1 + t^2 P2, 0 <= t <= 1
    // B'(t) = 2(1 - t) (P1 - P0) + 2t (P2 - P1)
    //       = 2 (P1 - P0) + 2t (P0 - 2P1 + P2)
    //
    // Let i = P1 - P0
    //     j = P2 - P1
    //
    // B'(t) = 2i - 2t (i - j)
    //
    // Let k = i - j
    //
    // B'(t) = 2i - 2kt
    //
    // Solve for B'(t) = 0
    //
    //     t = i / k
    //
    float i = p1 - p0;
    float j = p2 - p1;

    float k = i - j;

    static constexpr float epsilon = 0.1f;

    if (abs(k) < epsilon)
    {
      float t = 0.5;
      return CalculateQuadratic(t, p0, p1, p2);
    }

    float t = i / k;
    return CalculateQuadratic(t, p0, p1, p2);
  }

  KRYS_NODISCARD static FloatPoint CalculateQuadraticExtremity(const FloatPoint &currentPoint,
                                                               const FloatPoint &controlPoint,
                                                               const FloatPoint &endPoint) noexcept
  {
    float x = CalculateQuadraticExtremity(currentPoint.X(), controlPoint.X(), endPoint.X());
    float y = CalculateQuadraticExtremity(currentPoint.Y(), controlPoint.Y(), endPoint.Y());
    return {x, y};
  }

  void PathQuadCurveTo::ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &,
                                           FloatRect &boundingRect) const noexcept
  {
    auto extremity = CalculateQuadraticExtremity(currentPoint, ControlPoint, EndPoint);
    ExtendRect(boundingRect, currentPoint, extremity, EndPoint);
  }

  void PathQuadCurveTo::ApplyElements(const PathElementApplier &applier) const noexcept
  {
    applier({PathElement::Type::AddQuadCurveToPoint, {ControlPoint, EndPoint}});
  }

  void PathQuadCurveTo::Transform(const AffineTransform &transform) noexcept
  {
    ControlPoint = transform.MapPoint(ControlPoint);
    EndPoint = transform.MapPoint(EndPoint);
  }

  FloatPoint PathBezierCurveTo::CalculateEndPoint(const FloatPoint &, FloatPoint &) const noexcept
  {
    return EndPoint;
  }

  Maybe<FloatPoint> PathBezierCurveTo::TryGetEndPointWithoutContext() const noexcept
  {
    FloatPoint lastMoveToPoint;
    return CalculateEndPoint({}, lastMoveToPoint);
  }

  void PathBezierCurveTo::ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &,
                                                 FloatRect &boundingRect) const noexcept
  {
    ExtendRect(boundingRect, currentPoint, ControlPoint1, ControlPoint2, EndPoint);
  }

  KRYS_NODISCARD static float CalculateBezier(float t, float p0, float p1, float p2, float p3) noexcept
  {
    // Fallback to a value which won't affect the calculations.
    if (!(t >= 0 && t <= 1))
    {
      return p0;
    }

    float s = (1 - t);

    // B(t)  = (1 - t)^3 P0 + 3 (1 - t)^2 t P1 + 3 (1 -t) t^2 P2 + t^3 p3, 0 <= t <= 1
    return (s * s * s * p0) + (3 * s * s * t * p1) + (3 * s * t * t * p2) + (t * t * t * p3);
  }

  KRYS_NODISCARD static std::pair<float, float> CalculateBezierExtremities(float p0, float p1, float p2,
                                                                           float p3) noexcept
  {
    // B(t)  = (1 - t)^3 P0 + 3 (1 - t)^2 t P1 + 3 (1 -t) t^2 P2 + t^3 p3, 0 <= t <= 1
    // B'(t) = 3(1 - t)^2 (P1 - P0) + 6(1 - t)t (P2 - P1) + 3t^2 (P3 - P2)
    //
    // Let i = P1 - P0
    //     j = P2 - P1
    //     k = P3 - P2
    //
    // B'(t) = 3i(1 - t)^2 + 6j(1 - t)t + 3kt^2
    //       = (3i - 6j + 3k)t^2 + (-6i + 6j)t + 3i
    //
    // Let a = 3i - 6j + 3k
    //     b = -6i + 6j
    //     c = 3i
    //
    // B'(t) = at^2 + bt + c
    //
    // Solve for B'(t) = 0
    //
    //     t = (-b (+/-) sqrt(b^2 - 4ac))) / 2a
    //
    float i = p1 - p0;
    float j = p2 - p1;
    float k = p3 - p2;

    float a = 3 * i - 6 * j + 3 * k;
    float b = 6 * j - 6 * i;
    float c = 3 * i;

    static constexpr float epsilon = 0.1f;

    // Solve for the linear equation bt + c = 0.
    if (abs(a) < epsilon)
    {
      float t;
      // Get the t-coordinate of the quadartic curve vertex. It has to
      // be the mid-point between the current point and the end point.
      if (abs(b) < epsilon)
      {
        t = 0.5;
      }
      else
      {
        t = -c / b;
      }
      float s = CalculateBezier(t, p0, p1, p2, p3);
      return std::make_pair(s, s);
    }

    float sqrtPart = b * b - 4 * a * c;
    if (sqrtPart < 0)
    {
      return std::make_pair(p0, p0);
    }

    float t1 = (-b + sqrt(sqrtPart)) / (2 * a);
    float t2 = (-b - sqrt(sqrtPart)) / (2 * a);

    float s1 = CalculateBezier(t1, p0, p1, p2, p3);
    float s2 = CalculateBezier(t2, p0, p1, p2, p3);

    return std::make_pair(s1, s2);
  }

  KRYS_NODISCARD static std::pair<FloatPoint, FloatPoint>
    CalculateBezierExtremities(const FloatPoint &currentPoint, const FloatPoint &controlPoint1,
                               const FloatPoint &controlPoint2, const FloatPoint &endPoint) noexcept
  {
    auto x = CalculateBezierExtremities(currentPoint.X(), controlPoint1.X(), controlPoint2.X(), endPoint.X());
    auto y = CalculateBezierExtremities(currentPoint.Y(), controlPoint1.Y(), controlPoint2.Y(), endPoint.Y());

    return std::make_pair(FloatPoint(x.first, y.first), FloatPoint(x.second, y.second));
  }

  void PathBezierCurveTo::ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &,
                                             FloatRect &boundingRect) const noexcept
  {
    auto bezierExtremities = CalculateBezierExtremities(currentPoint, ControlPoint1, ControlPoint2, EndPoint);
    ExtendRect(boundingRect, currentPoint, bezierExtremities.first, bezierExtremities.second, EndPoint);
  }

  void PathBezierCurveTo::ApplyElements(const PathElementApplier &applier) const noexcept
  {
    applier({PathElement::Type::AddCurveToPoint, {ControlPoint1, ControlPoint2, EndPoint}});
  }

  void PathBezierCurveTo::Transform(const AffineTransform &transform) noexcept
  {
    ControlPoint1 = transform.MapPoint(ControlPoint1);
    ControlPoint2 = transform.MapPoint(ControlPoint2);
    EndPoint = transform.MapPoint(EndPoint);
  }

  KRYS_NODISCARD static float AngleOfLine(const FloatPoint &p1, const FloatPoint &p2) noexcept
  {
    if (abs(p1.X() - p2.X()) < 0.00001)
    {
      return p1.Y() - p2.Y() >= 0 ? std::numbers::pi_v<float> / 2 : 3 * std::numbers::pi_v<float> / 2;
    }
    return std::atan2(p1.Y() - p2.Y(), p1.X() - p2.X());
  }

  KRYS_NODISCARD static FloatPoint CalculateArcToEndPoint(const FloatPoint &currentPoint,
                                                          const FloatPoint &controlPoint1,
                                                          const FloatPoint &controlPoint2,
                                                          float radius) noexcept
  {
    float angle1 = AngleOfLine(currentPoint, controlPoint1);
    float angle2 = AngleOfLine(controlPoint1, controlPoint2);
    float angleBteweenLines = angle2 - angle1;

    if (abs(angleBteweenLines) < 0.00001 || abs(angleBteweenLines) >= std::numbers::pi_v<float> / 2)
    {
      return controlPoint1;
    }

    float adjacent = abs(radius / std::tan(angleBteweenLines / 2));

    float x = controlPoint1.X() + adjacent * std::cos(angle2);
    float y = controlPoint1.Y() - adjacent * std::sin(angle2);
    return {x, y};
  }

  FloatPoint PathArcTo::CalculateEndPoint(const FloatPoint &currentPoint, FloatPoint &) const noexcept
  {
    return CalculateArcToEndPoint(currentPoint, ControlPoint1, ControlPoint2, Radius);
  }

  Maybe<FloatPoint> PathArcTo::TryGetEndPointWithoutContext() const noexcept
  {
    return std::nullopt;
  }

  void PathArcTo::ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &,
                                         FloatRect &boundingRect) const noexcept
  {
    ExtendRect(boundingRect, currentPoint, ControlPoint1, ControlPoint2);
  }

  void PathArcTo::ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &,
                                     FloatRect &boundingRect) const noexcept
  {
    auto endPoint = CalculateArcToEndPoint(currentPoint, ControlPoint1, ControlPoint2, Radius);
    ExtendRect(boundingRect, currentPoint, ControlPoint1, endPoint);
  }

  FloatPoint PathArc::CalculateEndPoint(const FloatPoint &, FloatPoint &lastMoveToPoint) const noexcept
  {
    lastMoveToPoint = Center + FloatSize {Radius * std::cos(StartAngle), -Radius * std::sin(StartAngle)};
    return Center + FloatSize {Radius * std::cos(EndAngle), -Radius * std::sin(EndAngle)};
  }

  Maybe<FloatPoint> PathArc::TryGetEndPointWithoutContext() const noexcept
  {
    FloatPoint lastMoveToPoint;
    return CalculateEndPoint({}, lastMoveToPoint);
  }

  void PathArc::ExtendFastBoundingRect(const FloatPoint &, const FloatPoint &,
                                       FloatRect &boundingRect) const noexcept
  {
    auto minXMinYCorner = Center - FloatSize {Radius, Radius};
    auto maxXMaxYCorner = Center + FloatSize {Radius, Radius};
    boundingRect.Extend(minXMinYCorner, maxXMaxYCorner);
  }

  KRYS_NODISCARD static float AngleInClockwise(float angle, RotationDirection direction) noexcept
  {
    return direction == RotationDirection::Clockwise ? angle : angle - radiansPerTurnFloat;
  }

  void PathArc::ExtendBoundingRect(const FloatPoint &, const FloatPoint &,
                                   FloatRect &boundingRect) const noexcept
  {
    auto circleRect = FloatRect {Center - FloatSize {Radius, Radius}, Center + FloatSize {Radius, Radius}};

    if (EndAngle - StartAngle >= radiansPerTurnFloat)
    {
      boundingRect.Extend(circleRect.MinXMinYCorner(), circleRect.MaxXMaxYCorner());
      return;
    }

    float x1 = Center.X() + Radius * std::cos(StartAngle);
    float y1 = Center.Y() + Radius * std::sin(StartAngle);

    float x2 = Center.X() + Radius * std::cos(EndAngle);
    float y2 = Center.Y() + Radius * std::sin(EndAngle);

    float startAngle = this->StartAngle;
    float endAngle = this->EndAngle;

    if (x1 > x2)
    {
      std::swap(x1, x2);
    }

    if (y1 > y2)
    {
      std::swap(y1, y2);
    }

    if (Direction == RotationDirection::Counterclockwise)
    {
      std::swap(startAngle, endAngle);
      startAngle = AngleInClockwise(startAngle, Direction);
    }

    if (IsInRange(0.0f, startAngle, endAngle))
    {
      x2 = circleRect.MaxX();
    }

    if (IsInRange(AngleInClockwise(std::numbers::pi_v<float> / 2, Direction), startAngle, endAngle))
    {
      y2 = circleRect.MaxY();
    }

    if (IsInRange(AngleInClockwise(std::numbers::pi_v<float>, Direction), startAngle, endAngle))
    {
      x1 = circleRect.X();
    }

    if (IsInRange(AngleInClockwise(3 * std::numbers::pi_v<float> / 2, Direction), startAngle, endAngle))
    {
      y1 = circleRect.Y();
    }

    boundingRect.Extend({x1, y1});
    boundingRect.Extend({x2, y2});
  }

  FloatPoint PathClosedArc::CalculateEndPoint(const FloatPoint &, FloatPoint &lastMoveToPoint) const noexcept
  {
    lastMoveToPoint =
      Arc.Center + FloatSize {Arc.Radius * std::cos(Arc.StartAngle), -Arc.Radius * std::sin(Arc.StartAngle)};
    return lastMoveToPoint;
  }

  Maybe<FloatPoint> PathClosedArc::TryGetEndPointWithoutContext() const noexcept
  {
    FloatPoint lastMoveToPoint;
    return CalculateEndPoint({}, lastMoveToPoint);
  }

  void PathClosedArc::ExtendFastBoundingRect(const FloatPoint &currentPoint,
                                             const FloatPoint &lastMoveToPoint,
                                             FloatRect &boundingRect) const noexcept
  {
    Arc.ExtendFastBoundingRect(currentPoint, lastMoveToPoint, boundingRect);
  }

  void PathClosedArc::ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                         FloatRect &boundingRect) const noexcept
  {
    Arc.ExtendBoundingRect(currentPoint, lastMoveToPoint, boundingRect);
  }

  FloatPoint PathEllipse::CalculateEndPoint(const FloatPoint &, FloatPoint &lastMoveToPoint) const noexcept
  {
    lastMoveToPoint = Center + FloatSize {RadiusX * std::cos(StartAngle), -RadiusY * std::sin(StartAngle)};
    auto endPoint = Center + FloatSize {RadiusX * std::cos(EndAngle), -RadiusY * std::sin(EndAngle)};
    if (!Rotation)
    {
      return endPoint;
    }

    auto rotation = AffineTransform::MakeRotation(deg2rad(this->Rotation));
    lastMoveToPoint = rotation.MapPoint(lastMoveToPoint);
    return rotation.MapPoint(endPoint);
  }

  Maybe<FloatPoint> PathEllipse::TryGetEndPointWithoutContext() const noexcept
  {
    FloatPoint lastMoveToPoint;
    return CalculateEndPoint({}, lastMoveToPoint);
  }

  void PathEllipse::ExtendFastBoundingRect(const FloatPoint &, const FloatPoint &,
                                           FloatRect &boundingRect) const noexcept
  {
    auto minXMinYCorner = Center - FloatSize {RadiusX, RadiusY};
    auto maxXMaxYCorner = Center + FloatSize {RadiusX, RadiusY};

    if (!Rotation)
    {
      boundingRect.Extend(minXMinYCorner, maxXMaxYCorner);
      return;
    }

    auto rect = FloatRect {minXMinYCorner, maxXMaxYCorner};
    auto rotation = AffineTransform::MakeRotation(deg2rad(this->Rotation));
    rect = rotation.MapRect(rect);
    boundingRect.UniteEvenIfEmpty(rect);
  }

  void PathEllipse::ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &,
                                       FloatRect &boundingRect) const noexcept
  {
    // FIXME: Implement this function.
    boundingRect.Extend(currentPoint);
  }

  FloatPoint PathEllipseInRect::CalculateEndPoint(const FloatPoint &,
                                                  FloatPoint &lastMoveToPoint) const noexcept
  {
    lastMoveToPoint = Rect.Center() + FloatSize {Rect.Width() / 2, 0};
    return lastMoveToPoint;
  }

  Maybe<FloatPoint> PathEllipseInRect::TryGetEndPointWithoutContext() const noexcept
  {
    FloatPoint lastMoveToPoint;
    return CalculateEndPoint({}, lastMoveToPoint);
  }

  void PathEllipseInRect::ExtendFastBoundingRect(const FloatPoint &currentPoint,
                                                 const FloatPoint &lastMoveToPoint,
                                                 FloatRect &boundingRect) const noexcept
  {
    ExtendBoundingRect(currentPoint, lastMoveToPoint, boundingRect);
  }

  void PathEllipseInRect::ExtendBoundingRect(const FloatPoint &, const FloatPoint &,
                                             FloatRect &boundingRect) const noexcept
  {
    boundingRect.UniteEvenIfEmpty(Rect);
  }

  FloatPoint PathRect::CalculateEndPoint(const FloatPoint &, FloatPoint &lastMoveToPoint) const noexcept
  {
    lastMoveToPoint = Rect.Location();
    return lastMoveToPoint;
  }

  Maybe<FloatPoint> PathRect::TryGetEndPointWithoutContext() const noexcept
  {
    FloatPoint lastMoveToPoint;
    return CalculateEndPoint({}, lastMoveToPoint);
  }

  void PathRect::ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                        FloatRect &boundingRect) const noexcept
  {
    ExtendBoundingRect(currentPoint, lastMoveToPoint, boundingRect);
  }

  void PathRect::ExtendBoundingRect(const FloatPoint &, const FloatPoint &,
                                    FloatRect &boundingRect) const noexcept
  {
    boundingRect.UniteEvenIfEmpty(Rect);
  }

  FloatPoint PathRoundedRect::CalculateEndPoint(const FloatPoint &,
                                                FloatPoint &lastMoveToPoint) const noexcept
  {
    lastMoveToPoint = RoundedRect.Rect().Location();
    return lastMoveToPoint;
  }

  Maybe<FloatPoint> PathRoundedRect::TryGetEndPointWithoutContext() const noexcept
  {
    FloatPoint lastMoveToPoint;
    return CalculateEndPoint({}, lastMoveToPoint);
  }

  void PathRoundedRect::ExtendFastBoundingRect(const FloatPoint &currentPoint,
                                               const FloatPoint &lastMoveToPoint,
                                               FloatRect &boundingRect) const noexcept
  {
    ExtendBoundingRect(currentPoint, lastMoveToPoint, boundingRect);
  }

  void PathRoundedRect::ExtendBoundingRect(const FloatPoint &, const FloatPoint &,
                                           FloatRect &boundingRect) const noexcept
  {
    boundingRect.UniteEvenIfEmpty(RoundedRect.Rect());
  }

  FloatPoint PathContinuousRoundedRect::CalculateEndPoint(const FloatPoint &,
                                                          FloatPoint &lastMoveToPoint) const noexcept
  {
    lastMoveToPoint = Rect.Location();
    return lastMoveToPoint;
  }

  Maybe<FloatPoint> PathContinuousRoundedRect::TryGetEndPointWithoutContext() const noexcept
  {
    FloatPoint lastMoveToPoint;
    return CalculateEndPoint({}, lastMoveToPoint);
  }

  void PathContinuousRoundedRect::ExtendFastBoundingRect(const FloatPoint &currentPoint,
                                                         const FloatPoint &lastMoveToPoint,
                                                         FloatRect &boundingRect) const noexcept
  {
    ExtendBoundingRect(currentPoint, lastMoveToPoint, boundingRect);
  }

  void PathContinuousRoundedRect::ExtendBoundingRect(const FloatPoint &, const FloatPoint &,
                                                     FloatRect &boundingRect) const noexcept
  {
    boundingRect.UniteEvenIfEmpty(Rect);
  }

  FloatPoint PathDataLine::CalculateEndPoint(const FloatPoint &, FloatPoint &lastMoveToPoint) const noexcept
  {
    lastMoveToPoint = Start();
    return End();
  }

  Maybe<FloatPoint> PathDataLine::TryGetEndPointWithoutContext() const noexcept
  {
    FloatPoint lastMoveToPoint;
    return CalculateEndPoint({}, lastMoveToPoint);
  }

  void PathDataLine::ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                            FloatRect &boundingRect) const noexcept
  {
    ExtendBoundingRect(currentPoint, lastMoveToPoint, boundingRect);
  }

  void PathDataLine::ExtendBoundingRect(const FloatPoint &, const FloatPoint &,
                                        FloatRect &boundingRect) const noexcept
  {
    boundingRect.Extend(Start());
    boundingRect.Extend(End());
  }

  void PathDataLine::ApplyElements(const PathElementApplier &applier) const noexcept
  {
    applier({PathElement::Type::MoveToPoint, {Start()}});
    applier({PathElement::Type::AddLineToPoint, {End()}});
  }

  void PathDataLine::Transform(const AffineTransform &transform) noexcept
  {
    SetStart(transform.MapPoint(Start()));
    SetEnd(transform.MapPoint(End()));
  }

  FloatPoint PathDataQuadCurve::CalculateEndPoint(const FloatPoint &,
                                                  FloatPoint &lastMoveToPoint) const noexcept
  {
    lastMoveToPoint = Start;
    return EndPoint;
  }

  Maybe<FloatPoint> PathDataQuadCurve::TryGetEndPointWithoutContext() const noexcept
  {
    FloatPoint lastMoveToPoint;
    return CalculateEndPoint({}, lastMoveToPoint);
  }

  void PathDataQuadCurve::ExtendFastBoundingRect(const FloatPoint &, const FloatPoint &,
                                                 FloatRect &boundingRect) const noexcept
  {
    ExtendRect(boundingRect, Start, ControlPoint, EndPoint);
  }

  void PathDataQuadCurve::ExtendBoundingRect(const FloatPoint &, const FloatPoint &,
                                             FloatRect &boundingRect) const noexcept
  {
    auto extremity = CalculateQuadraticExtremity(Start, ControlPoint, EndPoint);
    ExtendRect(boundingRect, Start, extremity, EndPoint);
  }

  void PathDataQuadCurve::ApplyElements(const PathElementApplier &applier) const noexcept
  {
    applier({PathElement::Type::MoveToPoint, {Start}});
    applier({PathElement::Type::AddQuadCurveToPoint, {ControlPoint, EndPoint}});
  }

  void PathDataQuadCurve::Transform(const AffineTransform &transform) noexcept
  {
    Start = transform.MapPoint(Start);
    ControlPoint = transform.MapPoint(ControlPoint);
    EndPoint = transform.MapPoint(EndPoint);
  }

  FloatPoint PathDataBezierCurve::CalculateEndPoint(const FloatPoint &,
                                                    FloatPoint &lastMoveToPoint) const noexcept
  {
    lastMoveToPoint = Start;
    return EndPoint;
  }

  Maybe<FloatPoint> PathDataBezierCurve::TryGetEndPointWithoutContext() const noexcept
  {
    FloatPoint lastMoveToPoint;
    return CalculateEndPoint({}, lastMoveToPoint);
  }

  void PathDataBezierCurve::ExtendFastBoundingRect(const FloatPoint &, const FloatPoint &,
                                                   FloatRect &boundingRect) const noexcept
  {
    ExtendRect(boundingRect, Start, ControlPoint1, ControlPoint2, EndPoint);
  }

  void PathDataBezierCurve::ExtendBoundingRect(const FloatPoint &, const FloatPoint &,
                                               FloatRect &boundingRect) const noexcept
  {
    auto bezierExtremities = CalculateBezierExtremities(Start, ControlPoint1, ControlPoint2, EndPoint);
    ExtendRect(boundingRect, Start, bezierExtremities.first, bezierExtremities.second, EndPoint);
  }

  void PathDataBezierCurve::ApplyElements(const PathElementApplier &applier) const noexcept
  {
    applier({PathElement::Type::MoveToPoint, {Start}});
    applier({PathElement::Type::AddCurveToPoint, {ControlPoint1, ControlPoint2, EndPoint}});
  }

  void PathDataBezierCurve::Transform(const AffineTransform &transform) noexcept
  {
    Start = transform.MapPoint(Start);
    ControlPoint1 = transform.MapPoint(ControlPoint1);
    ControlPoint2 = transform.MapPoint(ControlPoint2);
    EndPoint = transform.MapPoint(EndPoint);
  }

  FloatPoint PathDataArc::CalculateEndPoint(const FloatPoint &, FloatPoint &lastMoveToPoint) const noexcept
  {
    lastMoveToPoint = Start;
    return CalculateArcToEndPoint(Start, ControlPoint1, ControlPoint2, Radius);
  }

  Maybe<FloatPoint> PathDataArc::TryGetEndPointWithoutContext() const noexcept
  {
    FloatPoint lastMoveToPoint;
    return CalculateEndPoint({}, lastMoveToPoint);
  }

  void PathDataArc::ExtendFastBoundingRect(const FloatPoint &, const FloatPoint &,
                                           FloatRect &boundingRect) const noexcept
  {
    ExtendRect(boundingRect, Start, ControlPoint1, ControlPoint2);
  }

  void PathDataArc::ExtendBoundingRect(const FloatPoint &, const FloatPoint &,
                                       FloatRect &boundingRect) const noexcept
  {
    auto endPoint = CalculateArcToEndPoint(Start, ControlPoint1, ControlPoint2, Radius);
    ExtendRect(boundingRect, Start, ControlPoint1, endPoint);
  }

  FloatPoint PathCloseSubpath::CalculateEndPoint(const FloatPoint &,
                                                 FloatPoint &lastMoveToPoint) const noexcept
  {
    return lastMoveToPoint;
  }

  Maybe<FloatPoint> PathCloseSubpath::TryGetEndPointWithoutContext() const noexcept
  {
    return std::nullopt;
  }

  void PathCloseSubpath::ExtendFastBoundingRect(const FloatPoint &, const FloatPoint &lastMoveToPoint,
                                                FloatRect &boundingRect) const noexcept
  {
    boundingRect.Extend(lastMoveToPoint);
  }

  void PathCloseSubpath::ExtendBoundingRect(const FloatPoint &, const FloatPoint &lastMoveToPoint,
                                            FloatRect &boundingRect) const noexcept
  {
    boundingRect.Extend(lastMoveToPoint);
  }

  void PathCloseSubpath::ApplyElements(const PathElementApplier &applier) const noexcept
  {
    applier({PathElement::Type::CloseSubpath, {}});
  }

  void PathCloseSubpath::Transform(const AffineTransform &) noexcept
  {
  }

  TextStream &operator<<(TextStream &ts, const PathMoveTo &data) noexcept
  {
    return ts << "move to " << data.Point;
  }

  TextStream &operator<<(TextStream &ts, const PathLineTo &data) noexcept
  {
    return ts << "add line to " << data.Point;
  }

  TextStream &operator<<(TextStream &ts, const PathQuadCurveTo &data) noexcept
  {
    return ts << "add quad curve to " << data.ControlPoint << ' ' << data.EndPoint;
  }

  TextStream &operator<<(TextStream &ts, const PathBezierCurveTo &data) noexcept
  {
    return ts << "add curve to " << data.ControlPoint1 << ' ' << data.ControlPoint2 << ' ' << data.EndPoint;
  }

  TextStream &operator<<(TextStream &ts, const PathArcTo &data) noexcept
  {
    return ts << "add arc to " << data.ControlPoint1 << ' ' << data.ControlPoint2 << ' ' << data.Radius;
  }

  TextStream &operator<<(TextStream &ts, const PathArc &data) noexcept
  {
    return ts << "add arc " << data.Center << ' ' << data.Radius << ' ' << data.StartAngle << ' '
              << data.EndAngle << ' ' << data.Direction;
  }

  TextStream &operator<<(TextStream &ts, const PathClosedArc &data) noexcept
  {
    return ts << "add closed arc " << data.Arc.Center << ' ' << data.Arc.Radius << ' ' << data.Arc.StartAngle
              << ' ' << data.Arc.EndAngle << ' ' << data.Arc.Direction;
  }

  TextStream &operator<<(TextStream &ts, const PathEllipse &data) noexcept
  {
    return ts << "add ellipse " << data.Center << ' ' << data.RadiusX << ' ' << data.RadiusY << ' '
              << data.Rotation << ' ' << data.StartAngle << ' ' << data.EndAngle << ' ' << data.Direction;
  }

  TextStream &operator<<(TextStream &ts, const PathEllipseInRect &data) noexcept
  {
    return ts << "add ellipse in rect " << data.Rect;
  }

  TextStream &operator<<(TextStream &ts, const PathRect &data) noexcept
  {
    return ts << "add rect " << data.Rect;
  }

  TextStream &operator<<(TextStream &ts, const PathRoundedRect &data) noexcept
  {
    return ts << "add rounded rect " << data.RoundedRect;
  }

  TextStream &operator<<(TextStream &ts, const PathContinuousRoundedRect &data) noexcept
  {
    return ts << "add continuous rounded rect " << data.Rect << ' ' << data.CornerWidth << ' '
              << data.CornerHeight;
  }

  TextStream &operator<<(TextStream &ts, const PathDataLine &data) noexcept
  {
    ts << "move to " << data.Start();
    ts << ", ";
    return ts << "add line to " << data.End();
  }

  TextStream &operator<<(TextStream &ts, const PathDataQuadCurve &data) noexcept
  {
    ts << "move to " << data.Start;
    ts << ", ";
    return ts << "add quad curve to " << data.ControlPoint << ' ' << data.EndPoint;
  }

  TextStream &operator<<(TextStream &ts, const PathDataBezierCurve &data) noexcept
  {
    ts << "move to " << data.Start;
    ts << ", ";
    return ts << "add curve to " << data.ControlPoint1 << ' ' << data.ControlPoint2 << ' ' << data.EndPoint;
  }

  TextStream &operator<<(TextStream &ts, const PathDataArc &data) noexcept
  {
    ts << "move to " << data.Start;
    ts << ", ";
    return ts << "add arc to " << data.ControlPoint1 << ' ' << data.ControlPoint2 << ' ' << data.Radius;
  }

  TextStream &operator<<(TextStream &ts, const PathCloseSubpath &) noexcept
  {
    return ts << "close subpath";
  }
}

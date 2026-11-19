#include "Krystal.Booey/Graphics/Geometry/GeometryUtilities.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include <numbers>

namespace krys::boo::tests
{
  using Catch::Matchers::WithinAbs;

  TEST_CASE("GeometryUtilities: distances and intersections", "[Graphics][GeometryUtilities]")
  {
    CHECK(EuclidianDistance(SizeF(3, 4)) == 5);
    CHECK(EuclidianDistance(PointF(1, 1), PointF(4, 5)) == 5);
    CHECK(MidPoint(PointF(0, 0), PointF(4, -2)) == PointF(2, -1));

    PointF intersection;
    REQUIRE(FindIntersection(PointF(0, 0), PointF(10, 10), PointF(0, 10), PointF(10, 0), intersection));
    CHECK(intersection == PointF(5, 5));
    // Parallel lines don't intersect, and the out parameter is untouched.
    CHECK_FALSE(FindIntersection(PointF(0, 0), PointF(10, 0), PointF(0, 1), PointF(10, 1), intersection));
    CHECK(intersection == PointF(5, 5));

    CHECK(DistanceToClosestSide(PointF(2, 5), SizeF(10, 20)) == 2);
    CHECK(DistanceToFarthestSide(PointF(2, 5), SizeF(10, 20)) == 15);
    CHECK(DistanceToClosestCorner(PointF(3, 4), SizeF(100, 100)) == 5);
    CHECK_THAT(DistanceToFarthestCorner(PointF(0, 0), SizeF(3, 4)), WithinAbs(5, 1e-6));

    auto sides = DistanceOfPointToSidesOfRect(RectF(0, 0, 10, 20), PointF(2, 5));
    CHECK(sides.Top() == 5);
    CHECK(sides.Right() == 8);
    CHECK(sides.Bottom() == 15);
    CHECK(sides.Left() == 2);

    auto vertices = VerticesForBox(RectF(0, 0, 10, 20), PointF(2, 5));
    CHECK(vertices[0] == PointF(-2, -5));
    CHECK(vertices[2] == PointF(8, 15));

    CHECK(EllipseContainsPoint(PointF(10, 10), SizeF(4, 2), PointF(13, 10)));
    CHECK(EllipseContainsPoint(PointF(10, 10), SizeF(4, 2), PointF(10, 12)));
    CHECK_FALSE(EllipseContainsPoint(PointF(10, 10), SizeF(4, 2), PointF(13, 11.5f)));
    CHECK_FALSE(EllipseContainsPoint(PointF(10, 10), SizeF(0, 2), PointF(10, 10)));
  }

  TEST_CASE("GeometryUtilities: rect helpers", "[Graphics][GeometryUtilities]")
  {
    RectI intRects[] = {RectI(0, 0, 10, 10), RectI(0, 0, 0, 5), RectI(20, 20, 5, 5)};
    CHECK(UnionRect(Span<const RectI>(intRects)) == RectI(0, 0, 25, 25));
    CHECK(UnionRectIgnoringZeroRects(Span<const RectI>(intRects)) == RectI(0, 0, 25, 25));
    RectI withEmpty[] = {RectI(10, 10, 10, 10), RectI(0, 0, 0, 5)};
    CHECK(UnionRect(Span<const RectI>(withEmpty)) == RectI(10, 10, 10, 10));
    CHECK(UnionRectIgnoringZeroRects(Span<const RectI>(withEmpty)) == RectI(0, 0, 20, 20));

    RectF floatRects[] = {RectF(10, 10, 10, 10), RectF(0, 0, 0, 5)};
    CHECK(UnionRect(Span<const RectF>(floatRects)) == RectF(10, 10, 10, 10));
    CHECK(UnionRectIgnoringZeroRects(Span<const RectF>(floatRects)) == RectF(0, 0, 20, 20));

    RectF src(0, 0, 10, 20);
    RectF dest(100, 100, 20, 10);
    CHECK(MapPoint(PointF(5, 10), src, dest) == PointF(110, 105));
    CHECK(MapPoint(PointF(5, 10), RectF(), dest) == PointF(5, 10));
    CHECK(MapRect(RectF(0, 10, 10, 10), src, dest) == RectF(100, 105, 20, 5));
    CHECK(MapRect(RectF(0, 10, 10, 10), RectF(), dest) == RectF());

    CHECK(LargestRectWithAspectRatioInsideRect(2, RectF(0, 0, 100, 100)) == RectF(0, 25, 100, 50));
    CHECK(LargestRectWithAspectRatioInsideRect(0.5f, RectF(0, 0, 100, 100)) == RectF(25, 0, 50, 100));
    CHECK(SmallestRectWithAspectRatioAroundRect(2, RectF(0, 0, 100, 100)) == RectF(-50, 0, 200, 100));
    CHECK(SmallestRectWithAspectRatioAroundRect(0.5f, RectF(0, 0, 100, 100)) == RectF(0, -50, 100, 200));
    CHECK(SizeWithAreaAndAspectRatio(200, 2) == SizeF(20, 10));

    CHECK(BoundsOfRotatingRect(RectF(3, 4, 0, 0)) == RectF(-5, -5, 10, 10));
    CHECK(ScaledRectAtOrigin(RectF(10, 10, 10, 10), 2, PointF(5, 5)) == RectF(15, 15, 20, 20));
  }

  TEST_CASE("GeometryUtilities: angles and rays", "[Graphics][GeometryUtilities]")
  {
    CHECK(ToPositiveAngle(-90) == 270);
    CHECK(ToPositiveAngle(450) == 90);
    CHECK(ToRelatedAcuteAngle(45) == 45);
    CHECK(ToRelatedAcuteAngle(135) == 45);
    CHECK(ToRelatedAcuteAngle(-45) == 135); // webkit.org/b/298890: angles over 270 aren't handled.
    CHECK_THAT(NormaliseAngleInRadians(-std::numbers::pi_v<float> / 2),
               WithinAbs(3 * std::numbers::pi_v<float> / 2, 1e-5));
    CHECK_THAT(NormaliseAngleInRadians(5 * std::numbers::pi_v<float>),
               WithinAbs(std::numbers::pi_v<float>, 1e-5));

    // A ray straight up from the middle of a 100x100 box hits the top after 50.
    RectF box(0, 0, 100, 100);
    PointF centre(50, 50);
    CHECK_THAT(LengthOfPointToSideOfIntersection(box, {centre, 0.0f}), WithinAbs(50, 1e-6));
    CHECK_THAT(AngleOfPointToSideOfIntersection(box, {centre, 0.0f}), WithinAbs(0, 1e-6));
    CHECK_THAT(LengthOfRayIntersectionWithBoundingBox(box, {centre, 0.0f}), WithinAbs(50, 1e-6));
    // At 45 degrees it hits the top-right corner.
    CHECK_THAT(LengthOfRayIntersectionWithBoundingBox(box, {centre, 45.0f}),
               WithinAbs(50 * std::numbers::sqrt2, 1e-4));
    // At 90 degrees it hits the right side.
    CHECK_THAT(LengthOfPointToSideOfIntersection(box, {PointF(80, 50), 90.0f}), WithinAbs(20, 1e-6));
    // A point outside the box has no length.
    CHECK(LengthOfPointToSideOfIntersection(box, {PointF(150, 50), 0.0f}) == 0);
  }

  TEST_CASE("GeometryUtilities: min and max", "[Graphics][GeometryUtilities]")
  {
    STATIC_CHECK(Min3(3, 1, 2) == 1);
    STATIC_CHECK(Max3(3, 1, 2) == 3);
    STATIC_CHECK(Min4(3, 1, 2, 0) == 0);
    STATIC_CHECK(Max4(3, 1, 4, 0) == 4);
  }
}

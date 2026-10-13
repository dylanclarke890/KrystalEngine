#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include <numbers>

namespace krys::boo::tests
{
  using Catch::Matchers::WithinAbs;

  TEST_CASE("GeometryUtilities: distances and intersections", "[Graphics][GeometryUtilities]")
  {
    CHECK(EuclidianDistance(FloatSize(3, 4)) == 5);
    CHECK(EuclidianDistance(FloatPoint(1, 1), FloatPoint(4, 5)) == 5);
    CHECK(MidPoint(FloatPoint(0, 0), FloatPoint(4, -2)) == FloatPoint(2, -1));

    FloatPoint intersection;
    REQUIRE(FindIntersection(FloatPoint(0, 0), FloatPoint(10, 10), FloatPoint(0, 10), FloatPoint(10, 0),
                             intersection));
    CHECK(intersection == FloatPoint(5, 5));
    // Parallel lines don't intersect, and the out parameter is untouched.
    CHECK_FALSE(FindIntersection(FloatPoint(0, 0), FloatPoint(10, 0), FloatPoint(0, 1), FloatPoint(10, 1),
                                 intersection));
    CHECK(intersection == FloatPoint(5, 5));

    CHECK(DistanceToClosestSide(FloatPoint(2, 5), FloatSize(10, 20)) == 2);
    CHECK(DistanceToFarthestSide(FloatPoint(2, 5), FloatSize(10, 20)) == 15);
    CHECK(DistanceToClosestCorner(FloatPoint(3, 4), FloatSize(100, 100)) == 5);
    CHECK_THAT(DistanceToFarthestCorner(FloatPoint(0, 0), FloatSize(3, 4)), WithinAbs(5, 1e-6));

    auto sides = DistanceOfPointToSidesOfRect(FloatRect(0, 0, 10, 20), FloatPoint(2, 5));
    CHECK(sides.Top() == 5);
    CHECK(sides.Right() == 8);
    CHECK(sides.Bottom() == 15);
    CHECK(sides.Left() == 2);

    auto vertices = VerticesForBox(FloatRect(0, 0, 10, 20), FloatPoint(2, 5));
    CHECK(vertices[0] == FloatPoint(-2, -5));
    CHECK(vertices[2] == FloatPoint(8, 15));

    CHECK(EllipseContainsPoint(FloatPoint(10, 10), FloatSize(4, 2), FloatPoint(13, 10)));
    CHECK(EllipseContainsPoint(FloatPoint(10, 10), FloatSize(4, 2), FloatPoint(10, 12)));
    CHECK_FALSE(EllipseContainsPoint(FloatPoint(10, 10), FloatSize(4, 2), FloatPoint(13, 11.5f)));
    CHECK_FALSE(EllipseContainsPoint(FloatPoint(10, 10), FloatSize(0, 2), FloatPoint(10, 10)));
  }

  TEST_CASE("GeometryUtilities: rect helpers", "[Graphics][GeometryUtilities]")
  {
    IntRect intRects[] = {IntRect(0, 0, 10, 10), IntRect(0, 0, 0, 5), IntRect(20, 20, 5, 5)};
    CHECK(UnionRect(Span<const IntRect>(intRects)) == IntRect(0, 0, 25, 25));
    CHECK(UnionRectIgnoringZeroRects(Span<const IntRect>(intRects)) == IntRect(0, 0, 25, 25));
    IntRect withEmpty[] = {IntRect(10, 10, 10, 10), IntRect(0, 0, 0, 5)};
    CHECK(UnionRect(Span<const IntRect>(withEmpty)) == IntRect(10, 10, 10, 10));
    CHECK(UnionRectIgnoringZeroRects(Span<const IntRect>(withEmpty)) == IntRect(0, 0, 20, 20));

    FloatRect floatRects[] = {FloatRect(10, 10, 10, 10), FloatRect(0, 0, 0, 5)};
    CHECK(UnionRect(Span<const FloatRect>(floatRects)) == FloatRect(10, 10, 10, 10));
    CHECK(UnionRectIgnoringZeroRects(Span<const FloatRect>(floatRects)) == FloatRect(0, 0, 20, 20));

    FloatRect src(0, 0, 10, 20);
    FloatRect dest(100, 100, 20, 10);
    CHECK(MapPoint(FloatPoint(5, 10), src, dest) == FloatPoint(110, 105));
    CHECK(MapPoint(FloatPoint(5, 10), FloatRect(), dest) == FloatPoint(5, 10));
    CHECK(MapRect(FloatRect(0, 10, 10, 10), src, dest) == FloatRect(100, 105, 20, 5));
    CHECK(MapRect(FloatRect(0, 10, 10, 10), FloatRect(), dest) == FloatRect());

    CHECK(LargestRectWithAspectRatioInsideRect(2, FloatRect(0, 0, 100, 100)) == FloatRect(0, 25, 100, 50));
    CHECK(LargestRectWithAspectRatioInsideRect(0.5f, FloatRect(0, 0, 100, 100)) == FloatRect(25, 0, 50, 100));
    CHECK(SmallestRectWithAspectRatioAroundRect(2, FloatRect(0, 0, 100, 100)) == FloatRect(-50, 0, 200, 100));
    CHECK(SmallestRectWithAspectRatioAroundRect(0.5f, FloatRect(0, 0, 100, 100))
          == FloatRect(0, -50, 100, 200));
    CHECK(SizeWithAreaAndAspectRatio(200, 2) == FloatSize(20, 10));

    CHECK(BoundsOfRotatingRect(FloatRect(3, 4, 0, 0)) == FloatRect(-5, -5, 10, 10));
    CHECK(ScaledRectAtOrigin(FloatRect(10, 10, 10, 10), 2, FloatPoint(5, 5)) == FloatRect(15, 15, 20, 20));
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
    FloatRect box(0, 0, 100, 100);
    FloatPoint centre(50, 50);
    CHECK_THAT(LengthOfPointToSideOfIntersection(box, {centre, 0.0f}), WithinAbs(50, 1e-6));
    CHECK_THAT(AngleOfPointToSideOfIntersection(box, {centre, 0.0f}), WithinAbs(0, 1e-6));
    CHECK_THAT(LengthOfRayIntersectionWithBoundingBox(box, {centre, 0.0f}), WithinAbs(50, 1e-6));
    // At 45 degrees it hits the top-right corner.
    CHECK_THAT(LengthOfRayIntersectionWithBoundingBox(box, {centre, 45.0f}),
               WithinAbs(50 * std::numbers::sqrt2, 1e-4));
    // At 90 degrees it hits the right side.
    CHECK_THAT(LengthOfPointToSideOfIntersection(box, {FloatPoint(80, 50), 90.0f}), WithinAbs(20, 1e-6));
    // A point outside the box has no length.
    CHECK(LengthOfPointToSideOfIntersection(box, {FloatPoint(150, 50), 0.0f}) == 0);
  }

  TEST_CASE("GeometryUtilities: min and max", "[Graphics][GeometryUtilities]")
  {
    STATIC_CHECK(Min3(3, 1, 2) == 1);
    STATIC_CHECK(Max3(3, 1, 2) == 3);
    STATIC_CHECK(Min4(3, 1, 2, 0) == 0);
    STATIC_CHECK(Max4(3, 1, 4, 0) == 4);
  }
}

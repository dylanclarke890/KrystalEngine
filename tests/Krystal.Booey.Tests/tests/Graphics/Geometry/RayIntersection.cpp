#include "Krystal.Booey/Graphics/Geometry/RayIntersection.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include <numbers>

namespace krys::boo::tests
{
  using Catch::Matchers::WithinAbs;

  TEST_CASE("RayIntersection: distances to a rect's sides", "[Graphics][RayIntersection]")
  {
    auto sides = DistanceOfPointToSidesOfRect(RectF(0, 0, 10, 20), PointF(2, 5));
    CHECK(sides.Top() == 5);
    CHECK(sides.Right() == 8);
    CHECK(sides.Bottom() == 15);
    CHECK(sides.Left() == 2);
  }

  TEST_CASE("RayIntersection: a ray's length to a rect's side", "[Graphics][RayIntersection]")
  {
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
}

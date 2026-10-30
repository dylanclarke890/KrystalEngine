#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"

namespace krys::boo::tests
{
  using Catch::Matchers::WithinAbs;
  using Catch::Matchers::WithinULP;

  static void ExpectRotatedRect(const FloatPoint &center, const FloatSize &size, float angle,
                                const RotatedRect &info)
  {
    CHECK_THAT(info.AngleInRadians, WithinAbs(angle, 0.001));
    CHECK_THAT(info.Center.X(), WithinAbs(center.X(), 0.001));
    CHECK_THAT(info.Center.Y(), WithinAbs(center.Y(), 0.001));
    CHECK_THAT(info.Size.Width(), WithinAbs(size.Width(), 0.001));
    CHECK_THAT(info.Size.Height(), WithinAbs(size.Height(), 0.001));
  }

  TEST_CASE("FloatQuad: RotatedBoundingRectForPoint", "[Graphics][FloatQuad]")
  {
    ExpectRotatedRect({1, -5}, {0, 0}, 0,
                      RotatedBoundingRectWithMinimumAngleOfRotation(
                        {FloatPoint(1, -5), FloatPoint(1, -5), FloatPoint(1, -5), FloatPoint(1, -5)}));
  }

  TEST_CASE("FloatQuad: RotatedBoundingRectForLineSegment", "[Graphics][FloatQuad]")
  {
    ExpectRotatedRect({0, -5}, {20, 0}, 0,
                      RotatedBoundingRectWithMinimumAngleOfRotation(
                        {FloatPoint(-10, -5), FloatPoint(10, -5), FloatPoint(10, -5), FloatPoint(-10, -5)}));
  }

  TEST_CASE("FloatQuad: RotatedBoundingRectForTriangle", "[Graphics][FloatQuad]")
  {
    ExpectRotatedRect({10, 0}, {40, 20}, 0,
                      RotatedBoundingRectWithMinimumAngleOfRotation(
                        {FloatPoint(-10, -10), FloatPoint(30, 0), FloatPoint(30, 0), FloatPoint(-10, 10)}));
  }

  TEST_CASE("FloatQuad: RotatedBoundingRectForTrapezoid", "[Graphics][FloatQuad]")
  {
    ExpectRotatedRect({25, 15}, {28.284f, 14.142f}, 5.498f,
                      RotatedBoundingRectWithMinimumAngleOfRotation(
                        {FloatPoint(10, 20), FloatPoint(20, 10), FloatPoint(40, 10), FloatPoint(20, 30)}));

    ExpectRotatedRect({20, 25}, {26.833f, 13.416f}, 1.107f,
                      RotatedBoundingRectWithMinimumAngleOfRotation(
                        {FloatPoint(20, 10), FloatPoint(30, 30), FloatPoint(20, 40), FloatPoint(10, 20)}));
  }

  TEST_CASE("FloatQuad: RotatedBoundingRectWithMinimumAngle", "[Graphics][FloatQuad]")
  {
    ExpectRotatedRect(
      {20, -14.95f}, {20, 10.1f}, 0,
      RotatedBoundingRectWithMinimumAngleOfRotation(
        {FloatPoint(10, -20), FloatPoint(30, -19.9f), FloatPoint(30, -9.9f), FloatPoint(10, -10)}, 0.01f));

    ExpectRotatedRect(
      {20, -15.05f}, {20, 10.1f}, 0,
      RotatedBoundingRectWithMinimumAngleOfRotation(
        {FloatPoint(10, -20), FloatPoint(30, -20.1f), FloatPoint(30, -10.1f), FloatPoint(10, -10)}, 0.01f));
  }

  static void CheckIsEmpty(bool expectation, FloatQuad &&quad)
  {
    CHECK(expectation == quad.IsEmpty());
  }

  TEST_CASE("FloatQuad: IsEmpty", "[Graphics][FloatQuad]")
  {
    CheckIsEmpty(true, {
                         // Line segment.
                         FloatPoint(0, 0),
                         FloatPoint(0, 0),
                         FloatPoint(0, 3),
                         FloatPoint(0, 3),
                       });

    CheckIsEmpty(false, {
                          // Triangle.
                          FloatPoint(0, 1),
                          FloatPoint(0, 0),
                          FloatPoint(3, 3),
                          FloatPoint(3, 3),
                        });

    CheckIsEmpty(false, {
                          // Another triangle.
                          FloatPoint(0, 0),
                          FloatPoint(0, 0),
                          FloatPoint(3, 3),
                          FloatPoint(0, 3),
                        });

    CheckIsEmpty(true, {
                         // Single point.
                         FloatPoint(100, 100),
                         FloatPoint(100, 100),
                         FloatPoint(100, 100),
                         FloatPoint(100, 100),
                       });

    CheckIsEmpty(true, {
                         // Two line segments.
                         FloatPoint(0, 0),
                         FloatPoint(100, 100),
                         FloatPoint(0, 0),
                         FloatPoint(50, -100),
                       });

    CheckIsEmpty(true, {
                         // 4 distinct but colinear points.
                         FloatPoint(-1, -1),
                         FloatPoint(0, 0),
                         FloatPoint(1, 1),
                         FloatPoint(3, 3),
                       });
  }

  TEST_CASE("FloatQuad: BoundingBox", "[Graphics][FloatQuad]")
  {
    FloatQuad quad(FloatPoint(2, 3), FloatPoint(5, 7), FloatPoint(11, 13), FloatPoint(17, 19));
    FloatRect rect = quad.BoundingBox();
    REQUIRE(rect.X() == 2);
    REQUIRE(rect.Y() == 3);
    REQUIRE(rect.Width() == 17 - 2);
    REQUIRE(rect.Height() == 19 - 3);
  }

  TEST_CASE("FloatQuad: BoundingBoxSaturateInf", "[Graphics][FloatQuad]")
  {
    constexpr double inf = std::numeric_limits<double>::infinity();
    FloatQuad quad(FloatPoint(static_cast<float>(-inf), 3), FloatPoint(5, static_cast<float>(inf)),
                   FloatPoint(11, 13), FloatPoint(17, 19));
    FloatRect rect = quad.BoundingBox();
    REQUIRE(rect.X() == std::numeric_limits<int>::min());
    REQUIRE(rect.Y() == 3.0f);
    REQUIRE(rect.Width() == 17.0f - std::numeric_limits<int>::min());
    REQUIRE(rect.Height() == static_cast<float>(std::numeric_limits<int>::max()) - 3.0f);
  }

  TEST_CASE("FloatQuad: BoundingBoxSaturateLarge", "[Graphics][FloatQuad]")
  {
    constexpr double large = static_cast<double>(std::numeric_limits<float>::max()) * 4;
    FloatQuad quad(FloatPoint(static_cast<float>(-large), 3), FloatPoint(5, static_cast<float>(large)),
                   FloatPoint(11, 13), FloatPoint(17, 19));
    FloatRect rect = quad.BoundingBox();
    REQUIRE(rect.X() == std::numeric_limits<int>::min());
    REQUIRE(rect.Y() == 3.0f);
    REQUIRE(rect.Width() == 17.0f - std::numeric_limits<int>::min());
    REQUIRE(rect.Height() == static_cast<float>(std::numeric_limits<int>::max()) - 3.0f);
  }

  TEST_CASE("FloatQuad: Containment and intersection", "[Graphics][FloatQuad]")
  {
    // A diamond centred on (10, 10).
    FloatQuad diamond(FloatPoint(10, 0), FloatPoint(20, 10), FloatPoint(10, 20), FloatPoint(0, 10));
    CHECK_FALSE(diamond.IsCounterclockwise());
    CHECK_FALSE(diamond.IsRectilinear());
    CHECK(diamond.Center() == FloatPoint(10, 10));
    CHECK(diamond.BoundingBox() == FloatRect(0, 0, 20, 20));
    CHECK(diamond.EnclosingBoundingBox() == IntRect(0, 0, 20, 20));

    CHECK(diamond.ContainsPoint(FloatPoint(10, 10)));
    CHECK(diamond.ContainsPoint(FloatPoint(5, 5))); // On an edge.
    CHECK_FALSE(diamond.ContainsPoint(FloatPoint(2, 2)));
    CHECK(diamond.ContainsQuad(FloatQuad(FloatRect(8, 8, 4, 4))));
    CHECK_FALSE(diamond.ContainsQuad(FloatQuad(FloatRect(0, 0, 4, 4))));

    CHECK(diamond.IntersectsRect(FloatRect(8, 8, 4, 4)));
    CHECK(diamond.IntersectsRect(FloatRect(-5, 8, 10, 4)));
    CHECK_FALSE(diamond.IntersectsRect(FloatRect(0, 0, 3, 3)));

    CHECK(diamond.IntersectsCircle(FloatPoint(10, 10), 1));
    CHECK(diamond.IntersectsCircle(FloatPoint(0, 0), 7.5f));
    CHECK_FALSE(diamond.IntersectsCircle(FloatPoint(0, 0), 7));
    CHECK(diamond.IntersectsEllipse(FloatPoint(-5, 10), FloatSize(6, 1)));
    CHECK_FALSE(diamond.IntersectsEllipse(FloatPoint(-5, 10), FloatSize(4, 1)));

    // The same quad in the other winding order.
    FloatQuad reversed(diamond.P1(), diamond.P4(), diamond.P3(), diamond.P2());
    CHECK(reversed.IsCounterclockwise());
    CHECK(reversed.IntersectsRect(FloatRect(8, 8, 4, 4)));
    CHECK_FALSE(reversed.IntersectsRect(FloatRect(0, 0, 3, 3)));

    FloatQuad rect(FloatRect(1, 2, 3, 4));
    CHECK(rect.IsRectilinear());
    CHECK(rect.P2() == FloatPoint(4, 2));
    CHECK(rect.P4() == FloatPoint(1, 6));
  }

  TEST_CASE("FloatQuad: Moving, scaling and lists", "[Graphics][FloatQuad]")
  {
    FloatQuad quad(FloatRect(1, 2, 3, 4));
    quad += FloatSize(1, 1);
    CHECK(quad == FloatQuad(FloatRect(2, 3, 3, 4)));
    quad -= FloatSize(1, 1);
    quad.Move(1, 0);
    CHECK(quad == FloatQuad(FloatRect(2, 2, 3, 4)));
    quad.Scale(2);
    CHECK(quad == FloatQuad(FloatRect(4, 4, 6, 8)));
    quad.Scale(0.5f, 1);
    CHECK(quad == FloatQuad(FloatRect(2, 4, 3, 8)));
    quad.SetP1(FloatPoint(0, 0));
    CHECK(quad.P1() == FloatPoint(0, 0));

    FloatQuad quads[] = {FloatQuad(FloatRect(0, 0, 1, 1)), FloatQuad(FloatRect(5, 5, 0, 2))};
    CHECK(BoundingBoxes(quads) == List<FloatRect> {FloatRect(0, 0, 1, 1), FloatRect(5, 5, 0, 2)});
    // Empty bounding boxes still extend the union.
    CHECK(UnitedBoundingBoxes(quads) == FloatRect(0, 0, 5, 7));
    CHECK(UnitedBoundingBoxes({}) == FloatRect());
  }
}

#include "Krystal.Booey/Graphics/Geometry/FloatQuad.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "Krystal.Booey/Graphics/Geometry/GeometryUtilities.hpp"

namespace krys::boo::tests
{
  using Catch::Matchers::WithinAbs;

  static void ExpectRotatedRect(const PointF &center, const SizeF &size, float angle, const RotatedRect &info)
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
                        {PointF(1, -5), PointF(1, -5), PointF(1, -5), PointF(1, -5)}));
  }

  TEST_CASE("FloatQuad: RotatedBoundingRectForLineSegment", "[Graphics][FloatQuad]")
  {
    ExpectRotatedRect({0, -5}, {20, 0}, 0,
                      RotatedBoundingRectWithMinimumAngleOfRotation(
                        {PointF(-10, -5), PointF(10, -5), PointF(10, -5), PointF(-10, -5)}));
  }

  TEST_CASE("FloatQuad: RotatedBoundingRectForTriangle", "[Graphics][FloatQuad]")
  {
    ExpectRotatedRect({10, 0}, {40, 20}, 0,
                      RotatedBoundingRectWithMinimumAngleOfRotation(
                        {PointF(-10, -10), PointF(30, 0), PointF(30, 0), PointF(-10, 10)}));
  }

  TEST_CASE("FloatQuad: RotatedBoundingRectForTrapezoid", "[Graphics][FloatQuad]")
  {
    ExpectRotatedRect({25, 15}, {28.284f, 14.142f}, 5.498f,
                      RotatedBoundingRectWithMinimumAngleOfRotation(
                        {PointF(10, 20), PointF(20, 10), PointF(40, 10), PointF(20, 30)}));

    ExpectRotatedRect({20, 25}, {26.833f, 13.416f}, 1.107f,
                      RotatedBoundingRectWithMinimumAngleOfRotation(
                        {PointF(20, 10), PointF(30, 30), PointF(20, 40), PointF(10, 20)}));
  }

  TEST_CASE("FloatQuad: RotatedBoundingRectWithMinimumAngle", "[Graphics][FloatQuad]")
  {
    ExpectRotatedRect({20, -14.95f}, {20, 10.1f}, 0,
                      RotatedBoundingRectWithMinimumAngleOfRotation(
                        {PointF(10, -20), PointF(30, -19.9f), PointF(30, -9.9f), PointF(10, -10)}, 0.01f));

    ExpectRotatedRect({20, -15.05f}, {20, 10.1f}, 0,
                      RotatedBoundingRectWithMinimumAngleOfRotation(
                        {PointF(10, -20), PointF(30, -20.1f), PointF(30, -10.1f), PointF(10, -10)}, 0.01f));
  }

  static void CheckIsEmpty(bool expectation, FloatQuad quad)
  {
    CHECK(expectation == quad.IsEmpty());
  }

  TEST_CASE("FloatQuad: IsEmpty", "[Graphics][FloatQuad]")
  {
    CheckIsEmpty(true, {
                         // Line segment.
                         PointF(0, 0),
                         PointF(0, 0),
                         PointF(0, 3),
                         PointF(0, 3),
                       });

    CheckIsEmpty(false, {
                          // Triangle.
                          PointF(0, 1),
                          PointF(0, 0),
                          PointF(3, 3),
                          PointF(3, 3),
                        });

    CheckIsEmpty(false, {
                          // Another triangle.
                          PointF(0, 0),
                          PointF(0, 0),
                          PointF(3, 3),
                          PointF(0, 3),
                        });

    CheckIsEmpty(true, {
                         // Single point.
                         PointF(100, 100),
                         PointF(100, 100),
                         PointF(100, 100),
                         PointF(100, 100),
                       });

    CheckIsEmpty(true, {
                         // Two line segments.
                         PointF(0, 0),
                         PointF(100, 100),
                         PointF(0, 0),
                         PointF(50, -100),
                       });

    CheckIsEmpty(true, {
                         // 4 distinct but colinear points.
                         PointF(-1, -1),
                         PointF(0, 0),
                         PointF(1, 1),
                         PointF(3, 3),
                       });
  }

  TEST_CASE("FloatQuad: BoundingBox", "[Graphics][FloatQuad]")
  {
    FloatQuad quad(PointF(2, 3), PointF(5, 7), PointF(11, 13), PointF(17, 19));
    RectF rect = quad.BoundingBox();
    REQUIRE(rect.X() == 2);
    REQUIRE(rect.Y() == 3);
    REQUIRE(rect.Width() == 17 - 2);
    REQUIRE(rect.Height() == 19 - 3);
  }

  TEST_CASE("FloatQuad: BoundingBoxSaturateInf", "[Graphics][FloatQuad]")
  {
    constexpr double inf = std::numeric_limits<double>::infinity();
    FloatQuad quad(PointF(static_cast<float>(-inf), 3), PointF(5, static_cast<float>(inf)), PointF(11, 13),
                   PointF(17, 19));
    RectF rect = quad.BoundingBox();
    REQUIRE(rect.X() == std::numeric_limits<int>::min());
    REQUIRE(rect.Y() == 3.0f);
    REQUIRE(rect.Width() == 17.0f - std::numeric_limits<int>::min());
    REQUIRE(rect.Height() == static_cast<float>(std::numeric_limits<int>::max()) - 3.0f);
  }

  TEST_CASE("FloatQuad: BoundingBoxSaturateLarge", "[Graphics][FloatQuad]")
  {
    constexpr double large = static_cast<double>(std::numeric_limits<float>::max()) * 4;
    FloatQuad quad(PointF(static_cast<float>(-large), 3), PointF(5, static_cast<float>(large)),
                   PointF(11, 13), PointF(17, 19));
    RectF rect = quad.BoundingBox();
    REQUIRE(rect.X() == std::numeric_limits<int>::min());
    REQUIRE(rect.Y() == 3.0f);
    REQUIRE(rect.Width() == 17.0f - std::numeric_limits<int>::min());
    REQUIRE(rect.Height() == static_cast<float>(std::numeric_limits<int>::max()) - 3.0f);
  }

  TEST_CASE("FloatQuad: Containment and intersection", "[Graphics][FloatQuad]")
  {
    // A diamond centred on (10, 10).
    FloatQuad diamond(PointF(10, 0), PointF(20, 10), PointF(10, 20), PointF(0, 10));
    CHECK_FALSE(diamond.IsCounterclockwise());
    CHECK_FALSE(diamond.IsRectilinear());
    CHECK(diamond.Center() == PointF(10, 10));
    CHECK(diamond.BoundingBox() == RectF(0, 0, 20, 20));
    CHECK(diamond.EnclosingBoundingBox() == RectI(0, 0, 20, 20));

    CHECK(diamond.ContainsPoint(PointF(10, 10)));
    CHECK(diamond.ContainsPoint(PointF(5, 5))); // On an edge.
    CHECK_FALSE(diamond.ContainsPoint(PointF(2, 2)));
    CHECK(diamond.ContainsQuad(FloatQuad(RectF(8, 8, 4, 4))));
    CHECK_FALSE(diamond.ContainsQuad(FloatQuad(RectF(0, 0, 4, 4))));

    CHECK(diamond.IntersectsRect(RectF(8, 8, 4, 4)));
    CHECK(diamond.IntersectsRect(RectF(-5, 8, 10, 4)));
    CHECK_FALSE(diamond.IntersectsRect(RectF(0, 0, 3, 3)));

    CHECK(diamond.IntersectsCircle(PointF(10, 10), 1));
    CHECK(diamond.IntersectsCircle(PointF(0, 0), 7.5f));
    CHECK_FALSE(diamond.IntersectsCircle(PointF(0, 0), 7));
    CHECK(diamond.IntersectsEllipse(PointF(-5, 10), SizeF(6, 1)));
    CHECK_FALSE(diamond.IntersectsEllipse(PointF(-5, 10), SizeF(4, 1)));

    // The same quad in the other winding order.
    FloatQuad reversed(diamond.P1(), diamond.P4(), diamond.P3(), diamond.P2());
    CHECK(reversed.IsCounterclockwise());
    CHECK(reversed.IntersectsRect(RectF(8, 8, 4, 4)));
    CHECK_FALSE(reversed.IntersectsRect(RectF(0, 0, 3, 3)));

    FloatQuad rect(RectF(1, 2, 3, 4));
    CHECK(rect.IsRectilinear());
    CHECK(rect.P2() == PointF(4, 2));
    CHECK(rect.P4() == PointF(1, 6));
  }

  TEST_CASE("FloatQuad: Moving, scaling and lists", "[Graphics][FloatQuad]")
  {
    FloatQuad quad(RectF(1, 2, 3, 4));
    quad += SizeF(1, 1);
    CHECK(quad == FloatQuad(RectF(2, 3, 3, 4)));
    quad -= SizeF(1, 1);
    quad.Move(1, 0);
    CHECK(quad == FloatQuad(RectF(2, 2, 3, 4)));
    quad.Scale(2);
    CHECK(quad == FloatQuad(RectF(4, 4, 6, 8)));
    quad.Scale(0.5f, 1);
    CHECK(quad == FloatQuad(RectF(2, 4, 3, 8)));
    quad.SetP1(PointF(0, 0));
    CHECK(quad.P1() == PointF(0, 0));

    FloatQuad quads[] = {FloatQuad(RectF(0, 0, 1, 1)), FloatQuad(RectF(5, 5, 0, 2))};
    CHECK(BoundingBoxes(quads) == List<RectF> {RectF(0, 0, 1, 1), RectF(5, 5, 0, 2)});
    // Empty bounding boxes still extend the union.
    CHECK(UnitedBoundingBoxes(quads) == RectF(0, 0, 5, 7));
    CHECK(UnitedBoundingBoxes({}) == RectF());
  }
}

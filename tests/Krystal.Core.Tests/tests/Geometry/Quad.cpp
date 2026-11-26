#include "Krystal.Core/Geometry/Quad.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

namespace krys::tests
{
  using Catch::Matchers::WithinAbs;

  static void CheckIsEmpty(bool expectation, QuadF quad)
  {
    CHECK(expectation == quad.IsEmpty());
  }

  TEST_CASE("QuadF: IsEmpty", "[Geometry][QuadF]")
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

  TEST_CASE("QuadF: BoundingBox", "[Geometry][QuadF]")
  {
    QuadF quad(PointF(2, 3), PointF(5, 7), PointF(11, 13), PointF(17, 19));
    RectF rect = quad.BoundingBox();
    REQUIRE(rect.X() == 2);
    REQUIRE(rect.Y() == 3);
    REQUIRE(rect.Width() == 17 - 2);
    REQUIRE(rect.Height() == 19 - 3);
  }

  TEST_CASE("QuadF: BoundingBoxSaturateInf", "[Geometry][QuadF]")
  {
    constexpr double inf = std::numeric_limits<double>::infinity();
    QuadF quad(PointF(static_cast<float>(-inf), 3), PointF(5, static_cast<float>(inf)), PointF(11, 13),
               PointF(17, 19));
    RectF rect = quad.BoundingBox();
    REQUIRE(rect.X() == std::numeric_limits<int>::min());
    REQUIRE(rect.Y() == 3.0f);
    REQUIRE(rect.Width() == 17.0f - std::numeric_limits<int>::min());
    REQUIRE(rect.Height() == static_cast<float>(std::numeric_limits<int>::max()) - 3.0f);
  }

  TEST_CASE("QuadF: BoundingBoxSaturateLarge", "[Geometry][QuadF]")
  {
    constexpr double large = static_cast<double>(std::numeric_limits<float>::max()) * 4;
    QuadF quad(PointF(static_cast<float>(-large), 3), PointF(5, static_cast<float>(large)), PointF(11, 13),
               PointF(17, 19));
    RectF rect = quad.BoundingBox();
    REQUIRE(rect.X() == std::numeric_limits<int>::min());
    REQUIRE(rect.Y() == 3.0f);
    REQUIRE(rect.Width() == 17.0f - std::numeric_limits<int>::min());
    REQUIRE(rect.Height() == static_cast<float>(std::numeric_limits<int>::max()) - 3.0f);
  }

  TEST_CASE("QuadF: Containment and intersection", "[Geometry][QuadF]")
  {
    // A diamond centred on (10, 10).
    QuadF diamond(PointF(10, 0), PointF(20, 10), PointF(10, 20), PointF(0, 10));
    CHECK_FALSE(diamond.IsCounterclockwise());
    CHECK_FALSE(diamond.IsRectilinear());
    CHECK(diamond.Center() == PointF(10, 10));
    CHECK(diamond.BoundingBox() == RectF(0, 0, 20, 20));
    CHECK(diamond.EnclosingBoundingBox() == RectI(0, 0, 20, 20));

    CHECK(diamond.ContainsPoint(PointF(10, 10)));
    CHECK(diamond.ContainsPoint(PointF(5, 5))); // On an edge.
    CHECK_FALSE(diamond.ContainsPoint(PointF(2, 2)));
    CHECK(diamond.ContainsQuad(QuadF(RectF(8, 8, 4, 4))));
    CHECK_FALSE(diamond.ContainsQuad(QuadF(RectF(0, 0, 4, 4))));

    CHECK(diamond.IntersectsRect(RectF(8, 8, 4, 4)));
    CHECK(diamond.IntersectsRect(RectF(-5, 8, 10, 4)));
    CHECK_FALSE(diamond.IntersectsRect(RectF(0, 0, 3, 3)));

    CHECK(diamond.IntersectsCircle(PointF(10, 10), 1));
    CHECK(diamond.IntersectsCircle(PointF(0, 0), 7.5f));
    CHECK_FALSE(diamond.IntersectsCircle(PointF(0, 0), 7));
    CHECK(diamond.IntersectsEllipse(PointF(-5, 10), SizeF(6, 1)));
    CHECK_FALSE(diamond.IntersectsEllipse(PointF(-5, 10), SizeF(4, 1)));

    // The same quad in the other winding order.
    QuadF reversed(diamond.P1(), diamond.P4(), diamond.P3(), diamond.P2());
    CHECK(reversed.IsCounterclockwise());
    CHECK(reversed.IntersectsRect(RectF(8, 8, 4, 4)));
    CHECK_FALSE(reversed.IntersectsRect(RectF(0, 0, 3, 3)));

    QuadF rect(RectF(1, 2, 3, 4));
    CHECK(rect.IsRectilinear());
    CHECK(rect.P2() == PointF(4, 2));
    CHECK(rect.P4() == PointF(1, 6));
  }

  TEST_CASE("QuadF: Moving, scaling and lists", "[Geometry][QuadF]")
  {
    QuadF quad(RectF(1, 2, 3, 4));
    quad += SizeF(1, 1);
    CHECK(quad == QuadF(RectF(2, 3, 3, 4)));
    quad -= SizeF(1, 1);
    quad.Move(1, 0);
    CHECK(quad == QuadF(RectF(2, 2, 3, 4)));
    quad.Scale(2);
    CHECK(quad == QuadF(RectF(4, 4, 6, 8)));
    quad.Scale(0.5f, 1);
    CHECK(quad == QuadF(RectF(2, 4, 3, 8)));
    quad.SetP1(PointF(0, 0));
    CHECK(quad.P1() == PointF(0, 0));

    QuadF quads[] = {QuadF(RectF(0, 0, 1, 1)), QuadF(RectF(5, 5, 0, 2))};
    CHECK(BoundingBoxes(quads) == List<RectF> {RectF(0, 0, 1, 1), RectF(5, 5, 0, 2)});
    // Empty bounding boxes still extend the union.
    CHECK(UnitedBoundingBoxes(quads) == RectF(0, 0, 5, 7));
    CHECK(UnitedBoundingBoxes({}) == RectF());
  }
}

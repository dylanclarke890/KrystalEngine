#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/IntRectHash.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"

namespace krys::boo::tests
{
  using Catch::Matchers::WithinAbs;
  using Catch::Matchers::WithinULP;

  static void TestGetAndSet(IntRect rect)
  {
    rect.SetX(1);
    CHECK(1 == rect.X());
    rect.SetY(2);
    CHECK(2 == rect.Y());
    rect.SetWidth(203);
    CHECK(203 == rect.Width());
    rect.SetHeight(73);
    CHECK(73 == rect.Height());
  }

  static void TestEmptyRect(const IntRect &rect)
  {
    CHECK(0 == rect.X());
    CHECK(0 == rect.Y());
    CHECK(0 == rect.Width());
    CHECK(0 == rect.Height());
    CHECK(0 == rect.MaxX());
    CHECK(0 == rect.MaxY());
    CHECK(rect.IsEmpty());
  }

  TEST_CASE("IntRect: DefaultConstruction", "[Graphics][IntRect]")
  {
    IntRect test;

    TestEmptyRect(test);

    TestGetAndSet(test);

    auto location = test.Location();
    CHECK(0 == location.X());
    CHECK(0 == location.Y());

    auto size = test.Size();
    CHECK(0 == size.Width());
    CHECK(0 == size.Height());
  }

  TEST_CASE("IntRect: ValueConstruction", "[Graphics][IntRect]")
  {
    IntRect test(10, 20, 100, 50);

    CHECK(10 == test.X());
    CHECK(20 == test.Y());
    CHECK(100 == test.Width());
    CHECK(50 == test.Height());
    CHECK(110 == test.MaxX());
    CHECK(70 == test.MaxY());
    CHECK_FALSE(test.IsEmpty());

    auto location = test.Location();
    CHECK(10 == location.X());
    CHECK(20 == location.Y());

    auto size = test.Size();
    CHECK(100 == size.Width());
    CHECK(50 == size.Height());
  }

  TEST_CASE("IntRect: PointSizeConstruction", "[Graphics][IntRect]")
  {
    IntPoint location(20, 30);
    IntSize size(100, 50);

    IntRect test(location, size);

    CHECK(20 == test.X());
    CHECK(30 == test.Y());
    CHECK(100 == test.Width());
    CHECK(50 == test.Height());
    CHECK(120 == test.MaxX());
    CHECK(80 == test.MaxY());
    CHECK_FALSE(test.IsEmpty());

    auto location2 = test.Location();
    CHECK(20 == location2.X());
    CHECK(30 == location2.Y());

    auto size2 = test.Size();
    CHECK(100 == size2.Width());
    CHECK(50 == size2.Height());
  }

  TEST_CASE("IntRect: FloatRectConstruction", "[Graphics][IntRect]")
  {
    FloatRect rect(20.0f, 30.0f, 150.0f, 300.0f);

    IntRect test(rect);

    CHECK(20 == test.X());
    CHECK(30 == test.Y());
    CHECK(150 == test.Width());
    CHECK(300 == test.Height());
    CHECK(170 == test.MaxX());
    CHECK(330 == test.MaxY());
    CHECK_FALSE(test.IsEmpty());

    auto location = test.Location();
    CHECK(20 == location.X());
    CHECK(30 == location.Y());

    auto size = test.Size();
    CHECK(150 == size.Width());
    CHECK(300 == size.Height());
  }

  TEST_CASE("IntRect: SetLocationAndSize", "[Graphics][IntRect]")
  {
    IntRect rect;

    TestEmptyRect(rect);

    IntPoint location(10, 20);

    rect.SetLocation(location);

    CHECK(10 == rect.X());
    CHECK(20 == rect.Y());
    CHECK(0 == rect.Width());
    CHECK(0 == rect.Height());
    CHECK(10 == rect.MaxX());
    CHECK(20 == rect.MaxY());
    CHECK(rect.IsEmpty());

    IntSize size(100, 200);

    rect.SetSize(size);

    CHECK(10 == rect.X());
    CHECK(20 == rect.Y());
    CHECK(100 == rect.Width());
    CHECK(200 == rect.Height());
    CHECK(110 == rect.MaxX());
    CHECK(220 == rect.MaxY());
    CHECK_FALSE(rect.IsEmpty());
  }

  TEST_CASE("IntRect: Center", "[Graphics][IntRect]")
  {
    IntRect rect(20, 40, 100, 200);

    auto center = rect.Center();

    CHECK(70 == center.X());
    CHECK(140 == center.Y());
  }

  TEST_CASE("IntRect: Move", "[Graphics][IntRect]")
  {
    IntRect rect(20, 30, 100, 200);

    IntSize delta(10, 20);

    rect.Move(delta);

    CHECK(30 == rect.X());
    CHECK(50 == rect.Y());

    IntPoint deltaPoint(-20, -40);

    rect.MoveBy(deltaPoint);

    CHECK(10 == rect.X());
    CHECK(10 == rect.Y());

    rect.Move(-10, 22);

    CHECK(0 == rect.X());
    CHECK(32 == rect.Y());
  }

  TEST_CASE("IntRect: Expand", "[Graphics][IntRect]")
  {
    IntRect rect(20, 30, 100, 200);

    IntSize size(100, 100);

    rect.Expand(size);

    CHECK(200 == rect.Width());
    CHECK(300 == rect.Height());

    rect.Expand(55, 22);

    CHECK(255 == rect.Width());
    CHECK(322 == rect.Height());

    IntSize size2(-10, -20);

    rect.Expand(size2);

    CHECK(245 == rect.Width());
    CHECK(302 == rect.Height());

    rect.Expand(-5, -2);

    CHECK(240 == rect.Width());
    CHECK(300 == rect.Height());
  }

  TEST_CASE("IntRect: Contract", "[Graphics][IntRect]")
  {
    IntRect rect(20, 30, 100, 200);

    IntSize size(50, 100);

    rect.Contract(size);

    CHECK(50 == rect.Width());
    CHECK(100 == rect.Height());

    rect.Contract(25, 22);

    CHECK(25 == rect.Width());
    CHECK(78 == rect.Height());

    IntSize size2(-10, -20);

    rect.Contract(size2);

    CHECK(35 == rect.Width());
    CHECK(98 == rect.Height());

    rect.Contract(-5, -2);

    CHECK(40 == rect.Width());
    CHECK(100 == rect.Height());
  }

  TEST_CASE("IntRect: ShiftXEdge", "[Graphics][IntRect]")
  {
    IntRect rect(20, 30, 100, 200);

    rect.ShiftXEdgeTo(77);

    CHECK(77 == rect.X());
    CHECK(120 == rect.MaxX());
    CHECK(30 == rect.Y());
    CHECK(230 == rect.MaxY());
    CHECK(43 == rect.Width());
    CHECK(200 == rect.Height());

    rect.ShiftMaxXEdgeTo(200);

    CHECK(77 == rect.X());
    CHECK(200 == rect.MaxX());
    CHECK(30 == rect.Y());
    CHECK(230 == rect.MaxY());
    CHECK(123 == rect.Width());
    CHECK(200 == rect.Height());
  }

  TEST_CASE("IntRect: ShiftYEdge", "[Graphics][IntRect]")
  {
    IntRect rect(20, 30, 100, 200);

    rect.ShiftYEdgeTo(59);

    CHECK(20 == rect.X());
    CHECK(120 == rect.MaxX());
    CHECK(59 == rect.Y());
    CHECK(230 == rect.MaxY());
    CHECK(100 == rect.Width());
    CHECK(171 == rect.Height());

    rect.ShiftMaxYEdgeTo(270);

    CHECK(20 == rect.X());
    CHECK(120 == rect.MaxX());
    CHECK(59 == rect.Y());
    CHECK(270 == rect.MaxY());
    CHECK(100 == rect.Width());
    CHECK(211 == rect.Height());
  }

  TEST_CASE("IntRect: Inflate", "[Graphics][IntRect]")
  {
    IntRect rect(20, 30, 100, 200);

    rect.InflateX(5);

    CHECK(15 == rect.X());
    CHECK(125 == rect.MaxX());

    rect.InflateY(4);

    CHECK(26 == rect.Y());
    CHECK(234 == rect.MaxY());

    rect.Inflate(10);

    CHECK(5 == rect.X());
    CHECK(135 == rect.MaxX());
    CHECK(16 == rect.Y());
    CHECK(244 == rect.MaxY());
  }

  TEST_CASE("IntRect: Corners", "[Graphics][IntRect]")
  {
    IntRect rect(20, 30, 100, 200);

    FloatPoint topLeft = rect.MinXMinYCorner();
    CHECK(20 == topLeft.X());
    CHECK(30 == topLeft.Y());

    FloatPoint topRight = rect.MaxXMinYCorner();
    CHECK(120 == topRight.X());
    CHECK(30 == topRight.Y());

    FloatPoint bottomLeft = rect.MinXMaxYCorner();
    CHECK(20 == bottomLeft.X());
    CHECK(230 == bottomLeft.Y());

    FloatPoint bottomRight = rect.MaxXMaxYCorner();
    CHECK(120 == bottomRight.X());
    CHECK(230 == bottomRight.Y());
  }

  TEST_CASE("IntRect: Contains", "[Graphics][IntRect]")
  {
    IntRect rect(20, 30, 100, 200);

    IntRect contained(30, 40, 50, 100);

    REQUIRE(rect.Contains(contained));

    IntRect outside(120, 230, 50, 100);

    REQUIRE_FALSE(rect.Contains(outside));

    IntRect intersects(10, 20, 90, 180);

    REQUIRE_FALSE(rect.Contains(intersects));

    IntPoint pointInside(60, 70);

    REQUIRE(rect.Contains(pointInside));

    IntPoint pointOutside(160, 270);

    REQUIRE_FALSE(rect.Contains(pointOutside));

    IntPoint pointOnLine(20, 30);

    REQUIRE(rect.Contains(pointOnLine));

    REQUIRE(rect.Contains(60, 70));
    REQUIRE_FALSE(rect.Contains(160, 270));
  }

  TEST_CASE("IntRect: Intersects", "[Graphics][IntRect]")
  {
    IntRect rect(20, 30, 100, 200);

    IntRect contained(30, 40, 50, 100);

    REQUIRE(rect.Intersects(contained));

    IntRect outside(120, 230, 50, 100);

    REQUIRE_FALSE(rect.Intersects(outside));

    IntRect intersects(10, 20, 90, 180);

    REQUIRE(rect.Intersects(intersects));
  }

  static void TestIntersectResult(const IntRect &rect)
  {
    CHECK(70 == rect.X());
    CHECK(120 == rect.MaxX());
    CHECK(80 == rect.Y());
    CHECK(230 == rect.MaxY());
  }

  TEST_CASE("IntRect: Intersect", "[Graphics][IntRect]")
  {
    IntRect rectA(20, 30, 100, 200);
    IntRect rectB(70, 80, 100, 200);

    rectA.Intersect(rectB);

    TestIntersectResult(rectA);

    IntRect rectC(20, 30, 100, 200);

    auto intersected = Intersection(rectC, rectB);

    TestIntersectResult(intersected);
  }

  static void TestUnitedRects(const IntRect &united)
  {
    CHECK(20 == united.X());
    CHECK(170 == united.MaxX());
    CHECK(30 == united.Y());
    CHECK(280 == united.MaxY());
  }

  TEST_CASE("IntRect: Unite", "[Graphics][IntRect]")
  {
    IntRect rectA(20, 30, 100, 200);
    IntRect rectB(70, 80, 100, 200);

    rectA.Unite(rectB);

    TestUnitedRects(rectA);

    IntRect rectC(20, 30, 100, 200);

    auto united = UnionRect(rectC, rectB);

    TestUnitedRects(united);
  }

  TEST_CASE("IntRect: Scale", "[Graphics][IntRect]")
  {
    IntRect rect(20, 30, 100, 200);

    rect.Scale(2.0f);

    CHECK(40 == rect.X());
    CHECK(240 == rect.MaxX());
    CHECK(60 == rect.Y());
    CHECK(460 == rect.MaxY());
  }

  TEST_CASE("IntRect: Transpose", "[Graphics][IntRect]")
  {
    IntRect rect(20, 30, 100, 200);

    auto transposed = rect.TransposedRect();

    CHECK(30 == transposed.X());
    CHECK(230 == transposed.MaxX());
    CHECK(20 == transposed.Y());
    CHECK(120 == transposed.MaxY());
  }

  static void CheckSubtractionResult1(const IntRect &rect)
  {
    CHECK(-10 == rect.X());
    CHECK(90 == rect.MaxX());
    CHECK(-10 == rect.Y());
    CHECK(90 == rect.MaxY());
    CHECK(100 == rect.Width());
    CHECK(100 == rect.Height());
  }

  static void CheckSubtractionResult2(const IntRect &rect)
  {
    CHECK(-40 == rect.X());
    CHECK(60 == rect.MaxX());
    CHECK(-50 == rect.Y());
    CHECK(50 == rect.MaxY());
    CHECK(100 == rect.Width());
    CHECK(100 == rect.Height());
  }

  TEST_CASE("IntRect: Subtraction", "[Graphics][IntRect]")
  {
    IntRect rect(10, 20, 100, 100);
    IntPoint rightSide(20, 30);

    rect -= rightSide;

    CheckSubtractionResult1(rect);

    auto rect2 = rect - IntPoint(30, 40);
    CheckSubtractionResult2(rect2);
  }

  TEST_CASE("IntRect: Equality", "[Graphics][IntRect]")
  {
    IntRect rect(10, 20, 100, 100);
    IntRect rect2(10, 20, 100, 100);
    IntRect rightSide(110, 20, 20, 100);

    REQUIRE(rect == rect2);
    REQUIRE_FALSE(rect != rect2);
    REQUIRE(rect != rightSide);
    REQUIRE_FALSE(rect == rightSide);
  }

  TEST_CASE("IntRect: Distances", "[Graphics][IntRect]")
  {
    IntRect rect(10, 20, 100, 100);

    CHECK(rect.DifferenceToPoint(IntPoint(50, 50)) == IntSize());
    CHECK(rect.DifferenceToPoint(IntPoint(0, 0)) == IntSize(10, 20));
    CHECK(rect.DifferenceToPoint(IntPoint(120, 130)) == IntSize(-10, -10));
    CHECK(rect.DistanceSquaredToPoint(IntPoint(0, 0)) == 500);
    CHECK(rect.DistanceSquaredToPoint(IntPoint(50, 50)) == 0);
  }

  TEST_CASE("IntRect: UniteIfNonZero", "[Graphics][IntRect]")
  {
    // A zero-sized rect is ignored, but an empty one with a non-zero dimension is united.
    IntRect rect(10, 20, 100, 100);
    rect.UniteIfNonZero(IntRect(500, 500, 0, 0));
    CHECK(rect == IntRect(10, 20, 100, 100));
    rect.UniteIfNonZero(IntRect(0, 0, 0, 10));
    CHECK(rect == IntRect(0, 0, 110, 120));

    IntRect zero;
    zero.UniteIfNonZero(IntRect(5, 5, 0, 10));
    CHECK(zero == IntRect(5, 5, 0, 10));

    // Unite ignores empty rects.
    IntRect united(10, 20, 100, 100);
    united.Unite(IntRect(0, 0, 0, 10));
    CHECK(united == IntRect(10, 20, 100, 100));
  }

  TEST_CASE("IntRect: Validity", "[Graphics][IntRect]")
  {
    constexpr int intMax = Limits<int>::max();

    CHECK(IntRect(10, 20, 100, 100).IsValid());
    CHECK_FALSE(IntRect(intMax - 10, 0, 20, 10).IsValid());
    CHECK_FALSE(IntRect(0, intMax - 10, 10, 20).IsValid());

    CHECK(IntRect(intMax - 10, intMax - 5, 20, 20).ToRectWithExtentsClippedToNumericLimits()
          == IntRect(intMax - 10, intMax - 5, 10, 5));
    CHECK(IntRect(10, 20, 100, 100).ToRectWithExtentsClippedToNumericLimits() == IntRect(10, 20, 100, 100));
  }

  TEST_CASE("IntRect: Shifting by and inflating by a size", "[Graphics][IntRect]")
  {
    IntRect rect(20, 30, 100, 200);
    rect.ShiftXEdgeBy(10);
    rect.ShiftYEdgeBy(20);
    CHECK(rect == IntRect(30, 50, 90, 180));
    rect.ShiftMaxXEdgeBy(-10);
    rect.ShiftMaxYEdgeBy(-200);
    CHECK(rect == IntRect(30, 50, 80, 0));

    rect.Inflate(IntSize(5, 10));
    CHECK(rect == IntRect(25, 40, 90, 20));
  }

  TEST_CASE("IntRect: Conversions", "[Graphics][IntRect]")
  {
    CHECK(IntRect(FloatRect(1.7f, -1.7f, 1e20f, -1e20f))
          == IntRect(1, -1, Limits<int>::max(), Limits<int>::min()));
    CHECK(IntRect(LayoutRect(LayoutUnit(1.75f), LayoutUnit(-1.75f), LayoutUnit(10.5f), LayoutUnit(2)))
          == IntRect(1, -1, 10, 2));

    CHECK(IntPoint(5, 500).ConstrainedWithin(IntRect(10, 20, 100, 100)) == IntPoint(10, 120));
  }

  TEST_CASE("IntRect: Hashing", "[Graphics][IntRect]")
  {
    CHECK(DefaultHash<IntRect>::Hash(IntRect(1, 2, 3, 4)) == DefaultHash<IntRect>::Hash(IntRect(1, 2, 3, 4)));
    CHECK(DefaultHash<IntRect>::Equal(IntRect(1, 2, 3, 4), IntRect(1, 2, 3, 4)));

    IntRect deleted;
    HashTraits<IntRect>::ConstructDeletedValue(deleted);
    CHECK(HashTraits<IntRect>::IsDeletedValue(deleted));
    CHECK_FALSE(HashTraits<IntRect>::IsDeletedValue(IntRect()));
  }
}

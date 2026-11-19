#include "Krystal.Core/Geometry/Rect.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "Krystal.Core/Geometry/RectHash.hpp"

namespace krys::tests
{
  static void TestGetAndSet(RectI rect)
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

  static void TestEmptyRect(const RectI &rect)
  {
    CHECK(0 == rect.X());
    CHECK(0 == rect.Y());
    CHECK(0 == rect.Width());
    CHECK(0 == rect.Height());
    CHECK(0 == rect.MaxX());
    CHECK(0 == rect.MaxY());
    CHECK(rect.IsEmpty());
  }

  TEST_CASE("RectI: DefaultConstruction", "[Geometry][RectI]")
  {
    RectI test;

    TestEmptyRect(test);

    TestGetAndSet(test);

    auto location = test.Location();
    CHECK(0 == location.X());
    CHECK(0 == location.Y());

    auto size = test.Size();
    CHECK(0 == size.Width());
    CHECK(0 == size.Height());
  }

  TEST_CASE("RectI: ValueConstruction", "[Geometry][RectI]")
  {
    RectI test(10, 20, 100, 50);

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

  TEST_CASE("RectI: PointSizeConstruction", "[Geometry][RectI]")
  {
    PointI location(20, 30);
    SizeI size(100, 50);

    RectI test(location, size);

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

  TEST_CASE("RectI: RectFConstruction", "[Geometry][RectI]")
  {
    RectF rect(20.0f, 30.0f, 150.0f, 300.0f);

    RectI test(rect);

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

  TEST_CASE("RectI: SetLocationAndSize", "[Geometry][RectI]")
  {
    RectI rect;

    TestEmptyRect(rect);

    PointI location(10, 20);

    rect.SetLocation(location);

    CHECK(10 == rect.X());
    CHECK(20 == rect.Y());
    CHECK(0 == rect.Width());
    CHECK(0 == rect.Height());
    CHECK(10 == rect.MaxX());
    CHECK(20 == rect.MaxY());
    CHECK(rect.IsEmpty());

    SizeI size(100, 200);

    rect.SetSize(size);

    CHECK(10 == rect.X());
    CHECK(20 == rect.Y());
    CHECK(100 == rect.Width());
    CHECK(200 == rect.Height());
    CHECK(110 == rect.MaxX());
    CHECK(220 == rect.MaxY());
    CHECK_FALSE(rect.IsEmpty());
  }

  TEST_CASE("RectI: Center", "[Geometry][RectI]")
  {
    RectI rect(20, 40, 100, 200);

    auto center = rect.Center();

    CHECK(70 == center.X());
    CHECK(140 == center.Y());
  }

  TEST_CASE("RectI: Move", "[Geometry][RectI]")
  {
    RectI rect(20, 30, 100, 200);

    SizeI delta(10, 20);

    rect.Move(delta);

    CHECK(30 == rect.X());
    CHECK(50 == rect.Y());

    PointI deltaPoint(-20, -40);

    rect.MoveBy(deltaPoint);

    CHECK(10 == rect.X());
    CHECK(10 == rect.Y());

    rect.Move(-10, 22);

    CHECK(0 == rect.X());
    CHECK(32 == rect.Y());
  }

  TEST_CASE("RectI: Expand", "[Geometry][RectI]")
  {
    RectI rect(20, 30, 100, 200);

    SizeI size(100, 100);

    rect.Expand(size);

    CHECK(200 == rect.Width());
    CHECK(300 == rect.Height());

    rect.Expand(55, 22);

    CHECK(255 == rect.Width());
    CHECK(322 == rect.Height());

    SizeI size2(-10, -20);

    rect.Expand(size2);

    CHECK(245 == rect.Width());
    CHECK(302 == rect.Height());

    rect.Expand(-5, -2);

    CHECK(240 == rect.Width());
    CHECK(300 == rect.Height());
  }

  TEST_CASE("RectI: Contract", "[Geometry][RectI]")
  {
    RectI rect(20, 30, 100, 200);

    SizeI size(50, 100);

    rect.Contract(size);

    CHECK(50 == rect.Width());
    CHECK(100 == rect.Height());

    rect.Contract(25, 22);

    CHECK(25 == rect.Width());
    CHECK(78 == rect.Height());

    SizeI size2(-10, -20);

    rect.Contract(size2);

    CHECK(35 == rect.Width());
    CHECK(98 == rect.Height());

    rect.Contract(-5, -2);

    CHECK(40 == rect.Width());
    CHECK(100 == rect.Height());
  }

  TEST_CASE("RectI: ShiftXEdge", "[Geometry][RectI]")
  {
    RectI rect(20, 30, 100, 200);

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

  TEST_CASE("RectI: ShiftYEdge", "[Geometry][RectI]")
  {
    RectI rect(20, 30, 100, 200);

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

  TEST_CASE("RectI: Inflate", "[Geometry][RectI]")
  {
    RectI rect(20, 30, 100, 200);

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

  TEST_CASE("RectI: Corners", "[Geometry][RectI]")
  {
    RectI rect(20, 30, 100, 200);

    PointF topLeft = rect.MinXMinYCorner();
    CHECK(20 == topLeft.X());
    CHECK(30 == topLeft.Y());

    PointF topRight = rect.MaxXMinYCorner();
    CHECK(120 == topRight.X());
    CHECK(30 == topRight.Y());

    PointF bottomLeft = rect.MinXMaxYCorner();
    CHECK(20 == bottomLeft.X());
    CHECK(230 == bottomLeft.Y());

    PointF bottomRight = rect.MaxXMaxYCorner();
    CHECK(120 == bottomRight.X());
    CHECK(230 == bottomRight.Y());
  }

  TEST_CASE("RectI: Contains", "[Geometry][RectI]")
  {
    RectI rect(20, 30, 100, 200);

    RectI contained(30, 40, 50, 100);

    REQUIRE(rect.Contains(contained));

    RectI outside(120, 230, 50, 100);

    REQUIRE_FALSE(rect.Contains(outside));

    RectI intersects(10, 20, 90, 180);

    REQUIRE_FALSE(rect.Contains(intersects));

    PointI pointInside(60, 70);

    REQUIRE(rect.Contains(pointInside));

    PointI pointOutside(160, 270);

    REQUIRE_FALSE(rect.Contains(pointOutside));

    PointI pointOnLine(20, 30);

    REQUIRE(rect.Contains(pointOnLine));

    REQUIRE(rect.Contains(60, 70));
    REQUIRE_FALSE(rect.Contains(160, 270));
  }

  TEST_CASE("RectI: Intersects", "[Geometry][RectI]")
  {
    RectI rect(20, 30, 100, 200);

    RectI contained(30, 40, 50, 100);

    REQUIRE(rect.Intersects(contained));

    RectI outside(120, 230, 50, 100);

    REQUIRE_FALSE(rect.Intersects(outside));

    RectI intersects(10, 20, 90, 180);

    REQUIRE(rect.Intersects(intersects));
  }

  static void TestIntersectResult(const RectI &rect)
  {
    CHECK(70 == rect.X());
    CHECK(120 == rect.MaxX());
    CHECK(80 == rect.Y());
    CHECK(230 == rect.MaxY());
  }

  TEST_CASE("RectI: Intersect", "[Geometry][RectI]")
  {
    RectI rectA(20, 30, 100, 200);
    RectI rectB(70, 80, 100, 200);

    rectA.Intersect(rectB);

    TestIntersectResult(rectA);

    RectI rectC(20, 30, 100, 200);

    auto intersected = Intersection(rectC, rectB);

    TestIntersectResult(intersected);
  }

  static void TestUnitedRects(const RectI &united)
  {
    CHECK(20 == united.X());
    CHECK(170 == united.MaxX());
    CHECK(30 == united.Y());
    CHECK(280 == united.MaxY());
  }

  TEST_CASE("RectI: Unite", "[Geometry][RectI]")
  {
    RectI rectA(20, 30, 100, 200);
    RectI rectB(70, 80, 100, 200);

    rectA.Unite(rectB);

    TestUnitedRects(rectA);

    RectI rectC(20, 30, 100, 200);

    auto united = UnionRect(rectC, rectB);

    TestUnitedRects(united);
  }

  TEST_CASE("RectI: Scale", "[Geometry][RectI]")
  {
    RectI rect(20, 30, 100, 200);

    rect.Scale(2.0f);

    CHECK(40 == rect.X());
    CHECK(240 == rect.MaxX());
    CHECK(60 == rect.Y());
    CHECK(460 == rect.MaxY());
  }

  TEST_CASE("RectI: Transpose", "[Geometry][RectI]")
  {
    RectI rect(20, 30, 100, 200);

    auto transposed = rect.TransposedRect();

    CHECK(30 == transposed.X());
    CHECK(230 == transposed.MaxX());
    CHECK(20 == transposed.Y());
    CHECK(120 == transposed.MaxY());
  }

  static void CheckSubtractionResult1(const RectI &rect)
  {
    CHECK(-10 == rect.X());
    CHECK(90 == rect.MaxX());
    CHECK(-10 == rect.Y());
    CHECK(90 == rect.MaxY());
    CHECK(100 == rect.Width());
    CHECK(100 == rect.Height());
  }

  static void CheckSubtractionResult2(const RectI &rect)
  {
    CHECK(-40 == rect.X());
    CHECK(60 == rect.MaxX());
    CHECK(-50 == rect.Y());
    CHECK(50 == rect.MaxY());
    CHECK(100 == rect.Width());
    CHECK(100 == rect.Height());
  }

  TEST_CASE("RectI: Subtraction", "[Geometry][RectI]")
  {
    RectI rect(10, 20, 100, 100);
    PointI rightSide(20, 30);

    rect -= rightSide;

    CheckSubtractionResult1(rect);

    auto rect2 = rect - PointI(30, 40);
    CheckSubtractionResult2(rect2);
  }

  TEST_CASE("RectI: Equality", "[Geometry][RectI]")
  {
    RectI rect(10, 20, 100, 100);
    RectI rect2(10, 20, 100, 100);
    RectI rightSide(110, 20, 20, 100);

    REQUIRE(rect == rect2);
    REQUIRE_FALSE(rect != rect2);
    REQUIRE(rect != rightSide);
    REQUIRE_FALSE(rect == rightSide);
  }

  TEST_CASE("RectI: Distances", "[Geometry][RectI]")
  {
    RectI rect(10, 20, 100, 100);

    CHECK(rect.DifferenceToPoint(PointI(50, 50)) == SizeI());
    CHECK(rect.DifferenceToPoint(PointI(0, 0)) == SizeI(10, 20));
    CHECK(rect.DifferenceToPoint(PointI(120, 130)) == SizeI(-10, -10));
    CHECK(rect.DistanceSquaredToPoint(PointI(0, 0)) == 500);
    CHECK(rect.DistanceSquaredToPoint(PointI(50, 50)) == 0);
  }

  TEST_CASE("RectI: UniteIfNonZero", "[Geometry][RectI]")
  {
    // A zero-sized rect is ignored, but an empty one with a non-zero dimension is united.
    RectI rect(10, 20, 100, 100);
    rect.UniteIfNonZero(RectI(500, 500, 0, 0));
    CHECK(rect == RectI(10, 20, 100, 100));
    rect.UniteIfNonZero(RectI(0, 0, 0, 10));
    CHECK(rect == RectI(0, 0, 110, 120));

    RectI zero;
    zero.UniteIfNonZero(RectI(5, 5, 0, 10));
    CHECK(zero == RectI(5, 5, 0, 10));

    // Unite ignores empty rects.
    RectI united(10, 20, 100, 100);
    united.Unite(RectI(0, 0, 0, 10));
    CHECK(united == RectI(10, 20, 100, 100));
  }

  TEST_CASE("RectI: Validity", "[Geometry][RectI]")
  {
    constexpr int intMax = Limits<int>::max();

    CHECK(RectI(10, 20, 100, 100).IsValid());
    CHECK_FALSE(RectI(intMax - 10, 0, 20, 10).IsValid());
    CHECK_FALSE(RectI(0, intMax - 10, 10, 20).IsValid());

    CHECK(RectI(intMax - 10, intMax - 5, 20, 20).ToRectWithExtentsClippedToNumericLimits()
          == RectI(intMax - 10, intMax - 5, 10, 5));
    CHECK(RectI(10, 20, 100, 100).ToRectWithExtentsClippedToNumericLimits() == RectI(10, 20, 100, 100));
  }

  TEST_CASE("RectI: Shifting by and inflating by a size", "[Geometry][RectI]")
  {
    RectI rect(20, 30, 100, 200);
    rect.ShiftXEdgeBy(10);
    rect.ShiftYEdgeBy(20);
    CHECK(rect == RectI(30, 50, 90, 180));
    rect.ShiftMaxXEdgeBy(-10);
    rect.ShiftMaxYEdgeBy(-200);
    CHECK(rect == RectI(30, 50, 80, 0));

    rect.Inflate(SizeI(5, 10));
    CHECK(rect == RectI(25, 40, 90, 20));
  }

  TEST_CASE("RectI: Conversions", "[Geometry][RectI]")
  {
    CHECK(RectI(RectF(1.7f, -1.7f, 1e20f, -1e20f)) == RectI(1, -1, Limits<int>::max(), Limits<int>::min()));

    CHECK(PointI(5, 500).ConstrainedWithin(RectI(10, 20, 100, 100)) == PointI(10, 120));
  }

  TEST_CASE("RectI: Hashing", "[Geometry][RectI]")
  {
    CHECK(DefaultHash<RectI>::Hash(RectI(1, 2, 3, 4)) == DefaultHash<RectI>::Hash(RectI(1, 2, 3, 4)));
    CHECK(DefaultHash<RectI>::Equal(RectI(1, 2, 3, 4), RectI(1, 2, 3, 4)));

    RectI deleted;
    HashTraits<RectI>::ConstructDeletedValue(deleted);
    CHECK(HashTraits<RectI>::IsDeletedValue(deleted));
    CHECK_FALSE(HashTraits<RectI>::IsDeletedValue(RectI()));
  }

  using Catch::Matchers::WithinULP;

  static void TestGetAndSet(RectF rect)
  {
    rect.SetX(1.1f);
    CHECK_THAT(rect.X(), WithinULP(1.1f, 4));
    rect.SetY(2.2f);
    CHECK_THAT(rect.Y(), WithinULP(2.2f, 4));
    rect.SetWidth(203.2f);
    CHECK_THAT(rect.Width(), WithinULP(203.2f, 4));
    rect.SetHeight(72.9f);
    CHECK_THAT(rect.Height(), WithinULP(72.9f, 4));
  }

  static void TestEmptyRect(const RectF &rect)
  {
    CHECK_THAT(rect.X(), WithinULP(static_cast<float>(0), 4));
    CHECK_THAT(rect.Y(), WithinULP(static_cast<float>(0), 4));
    CHECK_THAT(rect.Width(), WithinULP(static_cast<float>(0), 4));
    CHECK_THAT(rect.Height(), WithinULP(static_cast<float>(0), 4));
    CHECK_THAT(rect.MaxX(), WithinULP(static_cast<float>(0), 4));
    CHECK_THAT(rect.MaxY(), WithinULP(static_cast<float>(0), 4));
    CHECK(rect.IsEmpty());
    CHECK(rect.IsZero());
    CHECK_FALSE(rect.IsInfinite());
  }

  TEST_CASE("RectF: DefaultConstruction", "[Geometry][RectF]")
  {
    RectF test;

    TestEmptyRect(test);

    TestGetAndSet(test);

    auto location = test.Location();
    CHECK_THAT(location.X(), WithinULP(static_cast<float>(0), 4));
    CHECK_THAT(location.Y(), WithinULP(static_cast<float>(0), 4));

    auto size = test.Size();
    CHECK_THAT(size.Width(), WithinULP(static_cast<float>(0), 4));
    CHECK_THAT(size.Height(), WithinULP(static_cast<float>(0), 4));
  }

  TEST_CASE("RectF: ValueConstruction", "[Geometry][RectF]")
  {
    RectF test(10.0f, 20.0f, 100.0f, 50.0f);

    CHECK_THAT(test.X(), WithinULP(10.0f, 4));
    CHECK_THAT(test.Y(), WithinULP(20.0f, 4));
    CHECK_THAT(test.Width(), WithinULP(100.0f, 4));
    CHECK_THAT(test.Height(), WithinULP(50.0f, 4));
    CHECK_THAT(test.MaxX(), WithinULP(110.0f, 4));
    CHECK_THAT(test.MaxY(), WithinULP(70.0f, 4));
    CHECK_FALSE(test.IsEmpty());
    CHECK_FALSE(test.IsZero());
    CHECK_FALSE(test.IsInfinite());

    auto location = test.Location();
    CHECK_THAT(location.X(), WithinULP(10.0f, 4));
    CHECK_THAT(location.Y(), WithinULP(20.0f, 4));

    auto size = test.Size();
    CHECK_THAT(size.Width(), WithinULP(100.0f, 4));
    CHECK_THAT(size.Height(), WithinULP(50.0f, 4));
  }

  TEST_CASE("RectF: PointSizeConstruction", "[Geometry][RectF]")
  {
    PointF location(20.0f, 30.0f);
    SizeF size(100.0f, 50.0f);

    RectF test(location, size);

    CHECK_THAT(test.X(), WithinULP(20.0f, 4));
    CHECK_THAT(test.Y(), WithinULP(30.0f, 4));
    CHECK_THAT(test.Width(), WithinULP(100.0f, 4));
    CHECK_THAT(test.Height(), WithinULP(50.0f, 4));
    CHECK_THAT(test.MaxX(), WithinULP(120.0f, 4));
    CHECK_THAT(test.MaxY(), WithinULP(80.0f, 4));
    CHECK_FALSE(test.IsEmpty());
    CHECK_FALSE(test.IsZero());
    CHECK_FALSE(test.IsInfinite());

    auto location2 = test.Location();
    CHECK_THAT(location2.X(), WithinULP(20.0f, 4));
    CHECK_THAT(location2.Y(), WithinULP(30.0f, 4));

    auto size2 = test.Size();
    CHECK_THAT(size2.Width(), WithinULP(100.0f, 4));
    CHECK_THAT(size2.Height(), WithinULP(50.0f, 4));
  }

  TEST_CASE("RectF: TwoPointConstruction", "[Geometry][RectF]")
  {
    PointF point1(20.0f, 30.0f);
    PointF point2(150.0f, 250.0f);

    RectF test(point1, point2);

    CHECK_THAT(test.X(), WithinULP(20.0f, 4));
    CHECK_THAT(test.Y(), WithinULP(30.0f, 4));
    CHECK_THAT(test.Width(), WithinULP(130.0f, 4));
    CHECK_THAT(test.Height(), WithinULP(220.0f, 4));
    CHECK_THAT(test.MaxX(), WithinULP(150.0f, 4));
    CHECK_THAT(test.MaxY(), WithinULP(250.0f, 4));
    CHECK_FALSE(test.IsEmpty());
    CHECK_FALSE(test.IsZero());
    CHECK_FALSE(test.IsInfinite());

    auto location = test.Location();
    CHECK_THAT(location.X(), WithinULP(20.0f, 4));
    CHECK_THAT(location.Y(), WithinULP(30.0f, 4));

    auto size = test.Size();
    CHECK_THAT(size.Width(), WithinULP(130.0f, 4));
    CHECK_THAT(size.Height(), WithinULP(220.0f, 4));
  }

  TEST_CASE("RectF: RectIConstruction", "[Geometry][RectF]")
  {
    RectI rect(20, 30, 150, 300);

    RectF test(rect);

    CHECK_THAT(test.X(), WithinULP(20.0f, 4));
    CHECK_THAT(test.Y(), WithinULP(30.0f, 4));
    CHECK_THAT(test.Width(), WithinULP(150.0f, 4));
    CHECK_THAT(test.Height(), WithinULP(300.0f, 4));
    CHECK_THAT(test.MaxX(), WithinULP(170.0f, 4));
    CHECK_THAT(test.MaxY(), WithinULP(330.0f, 4));
    CHECK_FALSE(test.IsEmpty());
    CHECK_FALSE(test.IsZero());
    CHECK_FALSE(test.IsInfinite());

    auto location = test.Location();
    CHECK_THAT(location.X(), WithinULP(20.0f, 4));
    CHECK_THAT(location.Y(), WithinULP(30.0f, 4));

    auto size = test.Size();
    CHECK_THAT(size.Width(), WithinULP(150.0f, 4));
    CHECK_THAT(size.Height(), WithinULP(300.0f, 4));
  }

  TEST_CASE("RectF: SetLocationAndSize", "[Geometry][RectF]")
  {
    RectF rect;

    TestEmptyRect(rect);

    PointF location(10.0f, 20.0f);

    rect.SetLocation(location);

    CHECK_THAT(rect.X(), WithinULP(10.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(20.0f, 4));
    CHECK_THAT(rect.Width(), WithinULP(0.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(0.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(10.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(20.0f, 4));
    CHECK(rect.IsEmpty());
    CHECK(rect.IsZero());
    CHECK_FALSE(rect.IsInfinite());

    SizeF size(100.0f, 200.0f);

    rect.SetSize(size);

    CHECK_THAT(rect.X(), WithinULP(10.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(20.0f, 4));
    CHECK_THAT(rect.Width(), WithinULP(100.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(200.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(110.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(220.0f, 4));
    CHECK_FALSE(rect.IsEmpty());
    CHECK_FALSE(rect.IsZero());
    CHECK_FALSE(rect.IsInfinite());
  }

  TEST_CASE("RectF: Center", "[Geometry][RectF]")
  {
    RectF rect(20.0f, 30.0f, 100.0f, 200.0f);

    auto center = rect.Center();

    CHECK_THAT(center.X(), WithinULP(70.0f, 4));
    CHECK_THAT(center.Y(), WithinULP(130.0f, 4));
  }

  TEST_CASE("RectF: Move", "[Geometry][RectF]")
  {
    RectF rect(20.0f, 30.0f, 100.0f, 200.0f);

    SizeF delta(10.0f, 20.0f);

    rect.Move(delta);

    CHECK_THAT(rect.X(), WithinULP(30.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(50.0f, 4));

    PointF deltaPoint(-20.0f, -40.0f);

    rect.MoveBy(deltaPoint);

    CHECK_THAT(rect.X(), WithinULP(10.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(10.0f, 4));

    rect.Move(-10.0f, 22.0f);

    CHECK_THAT(rect.X(), WithinULP(0.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(32.0f, 4));
  }

  TEST_CASE("RectF: Expand", "[Geometry][RectF]")
  {
    RectF rect(20.0f, 30.0f, 100.0f, 200.0f);

    SizeF size(100.0f, 100.0f);

    rect.Expand(size);

    CHECK_THAT(rect.Width(), WithinULP(200.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(300.0f, 4));

    rect.Expand(55.0f, 22.0f);

    CHECK_THAT(rect.Width(), WithinULP(255.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(322.0f, 4));

    SizeF size2(-10.0f, -20.0f);

    rect.Expand(size2);

    CHECK_THAT(rect.Width(), WithinULP(245.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(302.0f, 4));

    rect.Expand(-5.0f, -2.0f);

    CHECK_THAT(rect.Width(), WithinULP(240.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(300.0f, 4));

    CHECK_FALSE(rect.IsInfinite());
  }

  TEST_CASE("RectF: Contract", "[Geometry][RectF]")
  {
    RectF rect(20.0f, 30.0f, 100.0f, 200.0f);

    SizeF size(50.0f, 100.0f);

    rect.Contract(size);

    CHECK_THAT(rect.Width(), WithinULP(50.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(100.0f, 4));

    rect.Contract(25.0f, 22.0f);

    CHECK_THAT(rect.Width(), WithinULP(25.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(78.0f, 4));

    SizeF size2(-10.0f, -20.0f);

    rect.Contract(size2);

    CHECK_THAT(rect.Width(), WithinULP(35.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(98.0f, 4));

    rect.Contract(-5.0f, -2.0f);

    CHECK_THAT(rect.Width(), WithinULP(40.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(100.0f, 4));

    CHECK_FALSE(rect.IsInfinite());
  }

  TEST_CASE("RectF: ShiftXEdge", "[Geometry][RectF]")
  {
    RectF rect(20.0f, 30.0f, 100.0f, 200.0f);

    rect.ShiftXEdgeTo(77.0f);

    CHECK_THAT(rect.X(), WithinULP(77.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(120.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(30.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(230.0f, 4));
    CHECK_THAT(rect.Width(), WithinULP(43.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(200.0f, 4));

    rect.ShiftMaxXEdgeTo(200.0f);

    CHECK_THAT(rect.X(), WithinULP(77.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(200.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(30.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(230.0f, 4));
    CHECK_THAT(rect.Width(), WithinULP(123.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(200.0f, 4));
  }

  TEST_CASE("RectF: ShiftYEdge", "[Geometry][RectF]")
  {
    RectF rect(20.0f, 30.0f, 100.0f, 200.0f);

    rect.ShiftYEdgeTo(59.0f);

    CHECK_THAT(rect.X(), WithinULP(20.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(120.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(59.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(230.0f, 4));
    CHECK_THAT(rect.Width(), WithinULP(100.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(171.0f, 4));

    rect.ShiftMaxYEdgeTo(270.0f);

    CHECK_THAT(rect.X(), WithinULP(20.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(120.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(59.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(270.0f, 4));
    CHECK_THAT(rect.Width(), WithinULP(100.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(211.0f, 4));
  }

  TEST_CASE("RectF: Inflate", "[Geometry][RectF]")
  {
    RectF rect(20.0f, 30.0f, 100.0f, 200.0f);

    rect.InflateX(5.0f);

    CHECK_THAT(rect.X(), WithinULP(15.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(125.0f, 4));

    rect.InflateY(4.0f);

    CHECK_THAT(rect.Y(), WithinULP(26.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(234.0f, 4));

    rect.Inflate(10.0f);

    CHECK_THAT(rect.X(), WithinULP(5.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(135.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(16.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(244.0f, 4));

    CHECK_FALSE(rect.IsInfinite());
  }

  TEST_CASE("RectF: Corners", "[Geometry][RectF]")
  {
    RectF rect(20.0f, 30.0f, 100.0f, 200.0f);

    PointF topLeft = rect.MinXMinYCorner();
    CHECK_THAT(topLeft.X(), WithinULP(20.0f, 4));
    CHECK_THAT(topLeft.Y(), WithinULP(30.0f, 4));

    PointF topRight = rect.MaxXMinYCorner();
    CHECK_THAT(topRight.X(), WithinULP(120.0f, 4));
    CHECK_THAT(topRight.Y(), WithinULP(30.0f, 4));

    PointF bottomLeft = rect.MinXMaxYCorner();
    CHECK_THAT(bottomLeft.X(), WithinULP(20.0f, 4));
    CHECK_THAT(bottomLeft.Y(), WithinULP(230.0f, 4));

    PointF bottomRight = rect.MaxXMaxYCorner();
    CHECK_THAT(bottomRight.X(), WithinULP(120.0f, 4));
    CHECK_THAT(bottomRight.Y(), WithinULP(230.0f, 4));
  }

  TEST_CASE("RectF: Contains", "[Geometry][RectF]")
  {
    RectF rect(20.0f, 30.0f, 100.0f, 200.0f);

    RectF contained(30.0f, 40.0f, 50.0f, 100.0f);

    REQUIRE(rect.Contains(contained));

    RectF outside(120.0f, 230.0f, 50.0f, 100.0f);

    REQUIRE_FALSE(rect.Contains(outside));

    RectF intersects(10.0f, 20.0f, 90.0f, 180.0f);

    REQUIRE_FALSE(rect.Contains(intersects));

    PointF pointInside(60.0f, 70.0f);

    REQUIRE(rect.Contains(pointInside));
    REQUIRE(rect.Contains(pointInside, RectF::ContainsMode::InsideButNotOnStroke));

    PointF pointOutside(160.0f, 270.0f);

    REQUIRE_FALSE(rect.Contains(pointOutside));
    REQUIRE_FALSE(rect.Contains(pointOutside, RectF::ContainsMode::InsideButNotOnStroke));

    PointF pointOnLine(20.0f, 30.0f);

    REQUIRE(rect.Contains(pointOnLine));
    REQUIRE_FALSE(rect.Contains(pointOutside, RectF::ContainsMode::InsideButNotOnStroke));

    REQUIRE(rect.Contains(60.0f, 70.0f));
    REQUIRE_FALSE(rect.Contains(160.0f, 270.0f));
  }

  TEST_CASE("RectF: Intersects", "[Geometry][RectF]")
  {
    RectF rect(20.0f, 30.0f, 100.0f, 200.0f);

    RectF contained(30.0f, 40.0f, 50.0f, 100.0f);

    REQUIRE(rect.Intersects(contained));

    RectF outside(120.0f, 230.0f, 50.0f, 100.0f);

    REQUIRE_FALSE(rect.Intersects(outside));

    RectF intersects(10.0f, 20.0f, 90.0f, 180.0f);

    REQUIRE(rect.Intersects(intersects));
  }

  static void TestIntersectResult(const RectF &rect)
  {
    CHECK_THAT(rect.X(), WithinULP(70.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(120.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(80.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(230.0f, 4));
  }

  TEST_CASE("RectF: Intersect", "[Geometry][RectF]")
  {
    RectF rectA(20.0f, 30.0f, 100.0f, 200.0f);
    RectF rectB(70.0f, 80.0f, 100.0f, 200.0f);

    rectA.Intersect(rectB);

    TestIntersectResult(rectA);

    RectF rectC(20.0f, 30.0f, 100.0f, 200.0f);

    auto intersected = Intersection(rectC, rectB);

    TestIntersectResult(intersected);
  }

  static void TestUnitedRects(const RectF &united)
  {
    CHECK_THAT(united.X(), WithinULP(20.0f, 4));
    CHECK_THAT(united.MaxX(), WithinULP(170.0f, 4));
    CHECK_THAT(united.Y(), WithinULP(30.0f, 4));
    CHECK_THAT(united.MaxY(), WithinULP(280.0f, 4));
  }

  TEST_CASE("RectF: Unite", "[Geometry][RectF]")
  {
    RectF rectA(20.0f, 30.0f, 100.0f, 200.0f);
    RectF rectB(70.0f, 80.0f, 100.0f, 200.0f);

    rectA.Unite(rectB);

    TestUnitedRects(rectA);

    RectF rectC(20.0f, 30.0f, 100.0f, 200.0f);

    auto united = UnionRect(rectC, rectB);

    TestUnitedRects(united);
  }

  TEST_CASE("RectF: Extend", "[Geometry][RectF]")
  {
    RectF rect(20.0f, 30.0f, 100.0f, 200.0f);
    PointF point(170.0f, 280.0f);

    rect.Extend(point);

    CHECK_THAT(rect.X(), WithinULP(20.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(170.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(30.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(280.0f, 4));
  }

  TEST_CASE("RectF: Overlaps", "[Geometry][RectF]")
  {
    RectF rect(20.0f, 30.0f, 100.0f, 200.0f);

    REQUIRE_FALSE(rect.OverlapsXRange(0.0f, 10.0f));
    REQUIRE(rect.OverlapsXRange(10.0f, 30.0f));
    REQUIRE(rect.OverlapsXRange(40.0f, 70.0f));
    REQUIRE(rect.OverlapsXRange(110.0f, 130.0f));
    REQUIRE_FALSE(rect.OverlapsXRange(140.0f, 600.0f));

    REQUIRE_FALSE(rect.OverlapsYRange(0.0f, 10.0f));
    REQUIRE(rect.OverlapsYRange(10.0f, 40.0f));
    REQUIRE(rect.OverlapsYRange(40.0f, 70.0f));
    REQUIRE(rect.OverlapsYRange(220.0f, 250.0f));
    REQUIRE_FALSE(rect.OverlapsYRange(250.0f, 600.0f));
  }

  TEST_CASE("RectF: Scale", "[Geometry][RectF]")
  {
    RectF rect(20.0f, 30.0f, 100.0f, 200.0f);

    rect.Scale(2.0f);

    CHECK_THAT(rect.X(), WithinULP(40.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(240.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(60.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(460.0f, 4));

    rect.Scale(0.5f, 2.0f);

    CHECK_THAT(rect.X(), WithinULP(20.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(120.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(120.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(920.0f, 4));

    rect.Scale(1.0f, 0.25f);

    CHECK_THAT(rect.X(), WithinULP(20.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(120.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(30.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(230.0f, 4));
  }

  TEST_CASE("RectF: Transpose", "[Geometry][RectF]")
  {
    RectF rect(20.0f, 30.0f, 100.0f, 200.0f);

    auto transposed = rect.TransposedRect();

    CHECK_THAT(transposed.X(), WithinULP(30.0f, 4));
    CHECK_THAT(transposed.MaxX(), WithinULP(230.0f, 4));
    CHECK_THAT(transposed.Y(), WithinULP(20.0f, 4));
    CHECK_THAT(transposed.MaxY(), WithinULP(120.0f, 4));
  }

  static void CheckAdditionResult1(const RectF &rect)
  {
    CHECK_THAT(rect.X(), WithinULP(10.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(130.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(20.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(220.0f, 4));
    CHECK_THAT(rect.Width(), WithinULP(120.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(200.0f, 4));
  }

  static void CheckAdditionResult2(const RectF &rect)
  {
    CHECK_THAT(rect.X(), WithinULP(10.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(250.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(20.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(240.0f, 4));
    CHECK_THAT(rect.Width(), WithinULP(240.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(220.0f, 4));
  }

  TEST_CASE("RectF: Addition", "[Geometry][RectF]")
  {
    RectF rect(10.0f, 20.0f, 100.0f, 100.0f);
    RectF rightSide(0.0f, 0.0f, 20.0f, 100.0f);
    RectF bottom(0.0f, 0.0f, 120.0f, 20.0f);

    auto combined = rect + rightSide;

    CheckAdditionResult1(combined);

    auto combined2 = combined + bottom;

    CheckAdditionResult2(combined2);

    rect += rightSide;
    rect += bottom;

    CheckAdditionResult2(rect);
  }

  TEST_CASE("RectF: Equality", "[Geometry][RectF]")
  {
    RectF rect(10.0f, 20.0f, 100.0f, 100.0f);
    RectF rect2(10.0f, 20.0f, 100.0f, 100.0f);
    RectF rightSide(110.0f, 20.0f, 20.0f, 100.0f);

    REQUIRE(rect == rect2);
    REQUIRE_FALSE(rect != rect2);
    REQUIRE(rect != rightSide);
    REQUIRE_FALSE(rect == rightSide);
  }

  TEST_CASE("RectF: InfiniteRect", "[Geometry][RectF]")
  {
    RectF infinite = RectF::InfiniteRect();

    CHECK_THAT(infinite.X(), WithinULP(static_cast<float>(-std::numeric_limits<float>::max() / 2), 4));
    CHECK_THAT(infinite.Y(), WithinULP(static_cast<float>(-std::numeric_limits<float>::max() / 2), 4));
    CHECK_THAT(infinite.Width(), WithinULP(static_cast<float>(std::numeric_limits<float>::max()), 4));
    CHECK_THAT(infinite.Height(), WithinULP(static_cast<float>(std::numeric_limits<float>::max()), 4));
    // The corner is half the lowest float, so the maximum corner doesn't overflow.
    CHECK(infinite.IsInfinite());
  }

  TEST_CASE("RectF: EnclosingAndRounding", "[Geometry][RectF]")
  {
    RectF rect(10.0f, 20.0f, 1024.3f, 768.3f);

    auto enclosed = EncloseRectToDevicePixels(rect, 1.0f);

    CHECK_THAT(enclosed.X(), WithinULP(10.0f, 4));
    CHECK_THAT(enclosed.Y(), WithinULP(20.0f, 4));
    CHECK_THAT(enclosed.MaxX(), WithinULP(1035.0f, 4));
    CHECK_THAT(enclosed.MaxY(), WithinULP(789.0f, 4));

    auto enclosed2 = EncloseRectToDevicePixels(rect, 2.0f);

    CHECK_THAT(enclosed2.X(), WithinULP(10.0f, 4));
    CHECK_THAT(enclosed2.Y(), WithinULP(20.0f, 4));
    CHECK_THAT(enclosed2.MaxX(), WithinULP(1034.5f, 4));
    CHECK_THAT(enclosed2.MaxY(), WithinULP(788.5f, 4));
  }

  TEST_CASE("RectF: EnclosingRectI", "[Geometry][RectF]")
  {
    RectF rect(10.0f, 20.0f, 1024.3f, 768.6f);

    auto enclosed = EnclosingRectI(rect);

    CHECK(10 == enclosed.X());
    CHECK(20 == enclosed.Y());
    CHECK(1'035 == enclosed.MaxX());
    CHECK(789 == enclosed.MaxY());

    RectF maxRectI(INT_MIN, INT_MIN, 0, 0);
    maxRectI.ShiftMaxXEdgeTo(30);
    maxRectI.ShiftMaxYEdgeTo(30);

    auto enclosed2 = EnclosingRectI(maxRectI);

    CHECK(INT_MIN == enclosed2.X());
    CHECK(INT_MIN == enclosed2.Y());
    CHECK(INT_MAX == enclosed2.Width());
    CHECK(INT_MAX == enclosed2.Height());

    RectF smallDimensionsRect(42.5f, 84.5f, std::numeric_limits<float>::epsilon(),
                              std::numeric_limits<float>::epsilon());
    CHECK(RectI(42, 84, 1, 1) == EnclosingRectI(smallDimensionsRect));

    RectF integralRect(100, 150, 200, 350);
    CHECK(RectI(100, 150, 200, 350) == EnclosingRectI(integralRect));

    RectF fractionalPosRect(100.6f, 150.8f, 200, 350);
    CHECK(RectI(100, 150, 201, 351) == EnclosingRectI(fractionalPosRect));

    RectF fractionalDimensionsRect(100, 150, 200.6f, 350.4f);
    CHECK(RectI(100, 150, 201, 351) == EnclosingRectI(fractionalDimensionsRect));

    RectF fractionalBothRect1(100.6f, 150.8f, 200.4f, 350.2f);
    CHECK(RectI(100, 150, 201, 351) == EnclosingRectI(fractionalBothRect1));

    RectF fractionalBothRect2(100.6f, 150.8f, 200.3f, 350.3f);
    CHECK(RectI(100, 150, 201, 352) == EnclosingRectI(fractionalBothRect2));

    RectF fractionalBothRect3(100.6f, 150.8f, 200.5f, 350.3f);
    CHECK(RectI(100, 150, 202, 352) == EnclosingRectI(fractionalBothRect3));

    RectF fractionalNegposRect1(-100.4f, -150.8f, 200, 350);
    CHECK(RectI(-101, -151, 201, 351) == EnclosingRectI(fractionalNegposRect1));

    RectF fractionalNegposRect2(-100.4f, -150.8f, 199.4f, 350.3f);
    CHECK(RectI(-101, -151, 200, 351) == EnclosingRectI(fractionalNegposRect2));

    RectF fractionalNegposRect3(-100.6f, -150.8f, 199.6f, 350.3f);
    CHECK(RectI(-101, -151, 201, 351) == EnclosingRectI(fractionalNegposRect3));

    constexpr int intMin = std::numeric_limits<int>::min();
    constexpr int intMax = std::numeric_limits<int>::max();
    RectF maxRect(-std::numeric_limits<float>::max() / 2, -std::numeric_limits<float>::max() / 2,
                  std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    CHECK(RectI(intMin, intMin, intMax, intMax) == EnclosingRectI(maxRect));
  }

  TEST_CASE("RectF: RoundedRectI", "[Geometry][RectF]")
  {
    RectF rect(10.0f, 20.0f, 1024.3f, 768.6f);

    auto enclosed = RoundedRectI(rect);

    CHECK(10 == enclosed.X());
    CHECK(20 == enclosed.Y());
    CHECK(1'034 == enclosed.MaxX());
    CHECK(789 == enclosed.MaxY());
  }

  TEST_CASE("RectF: Markable", "[Geometry][RectF]")
  {
    RectF rect(10.0f, 20.0f, 1024.3f, 768.6f);
    Markable<RectF> optional;
    CHECK_FALSE(optional);
    optional = rect;
    CHECK((optional.value_or(RectF {})) == rect);
    optional = RectF::NanRect();
    CHECK_FALSE(optional);
  }

  TEST_CASE("RectF: Intersection variants", "[Geometry][RectF]")
  {
    // Touching rects don't intersect, but do inclusively.
    RectF a(0, 0, 10, 10);
    RectF b(10, 0, 10, 10);
    CHECK_FALSE(a.Intersects(b));
    CHECK(a.InclusivelyIntersects(b));
    CHECK_FALSE(a.InclusivelyIntersects(RectF(10.5f, 0, 10, 10)));
    // An empty rect still inclusively intersects.
    CHECK(a.InclusivelyIntersects(RectF(5, 5, 0, 0)));
    CHECK_FALSE(a.Intersects(RectF(5, 5, 0, 0)));

    RectF edge = a;
    CHECK(edge.EdgeInclusiveIntersect(b));
    CHECK(edge == RectF(10, 0, 0, 10));

    RectF apart = a;
    CHECK_FALSE(apart.EdgeInclusiveIntersect(RectF(20, 20, 5, 5)));
    CHECK(apart == RectF());

    RectF disjoint = a;
    disjoint.Intersect(RectF(20, 20, 5, 5));
    CHECK(disjoint == RectF());
  }

  TEST_CASE("RectF: Union variants", "[Geometry][RectF]")
  {
    RectF rect(10, 10, 10, 10);
    rect.Unite(RectF(100, 100, 0, 5));
    CHECK(rect == RectF(10, 10, 10, 10));
    rect.UniteEvenIfEmpty(RectF(100, 100, 0, 5));
    CHECK(rect == RectF(10, 10, 90, 95));

    RectF nonZero(10, 10, 10, 10);
    nonZero.UniteIfNonZero(RectF(0, 0, 0, 0));
    CHECK(nonZero == RectF(10, 10, 10, 10));
    nonZero.UniteIfNonZero(RectF(0, 0, 0, 5));
    CHECK(nonZero == RectF(0, 0, 20, 20));

    RectF extended(10, 10, 10, 10);
    extended.Extend(PointF(0, 5), PointF(30, 25));
    CHECK(extended == RectF(0, 5, 30, 20));
  }

  TEST_CASE("RectF: Contains modes", "[Geometry][RectF]")
  {
    RectF rect(10, 10, 10, 10);
    CHECK(rect.Contains(PointF(10, 15)));
    CHECK(rect.Contains(PointF(10, 15), RectF::ContainsMode::InsideOrOnStroke));
    CHECK_FALSE(rect.Contains(PointF(10, 15), RectF::ContainsMode::InsideButNotOnStroke));
    CHECK(rect.Contains(PointF(15, 15), RectF::ContainsMode::InsideButNotOnStroke));
  }

  TEST_CASE("RectF: Edges", "[Geometry][RectF]")
  {
    RectF rect(10, 10, 10, 10);
    rect.ShiftEdgesTo(1, 2, 5, 10);
    CHECK(rect == RectF(1, 2, 4, 8));
    rect.ShiftXEdgeBy(1);
    rect.ShiftYEdgeBy(1);
    rect.ShiftMaxXEdgeBy(2);
    rect.ShiftMaxYEdgeBy(-10);
    CHECK(rect == RectF(2, 3, 5, 0));

    rect.Inflate(1, 2, 3, 4);
    CHECK(rect == RectF(1, 1, 9, 6));
    rect.Inflate(SizeF(1, 1));
    CHECK(rect == RectF(0, 0, 11, 8));
  }

  TEST_CASE("RectF: Special rects and helpers", "[Geometry][RectF]")
  {
    CHECK(RectF::InfiniteRect().IsInfinite());
    CHECK(RectF::SmallestRect().IsSmallest());
    CHECK_FALSE(RectF().IsSmallest());
    CHECK(RectF::NanRect().IsNaN());
    CHECK_FALSE(RectF().IsNaN());

    CHECK(NormaliseRect(RectF(10, 20, -5, -10)) == RectF(5, 10, 5, 10));
    CHECK(NormaliseRect(RectF(10, 20, 5, 10)) == RectF(10, 20, 5, 10));

    CHECK(EnclosingRectIPreservingEmptyRects(RectF(1.5f, 2.5f, 0, 3)) == RectI(1, 2, 0, 0));
    CHECK(EnclosingRectIPreservingEmptyRects(RectF(1.5f, 2.5f, 1, 3)) == RectI(1, 2, 2, 4));

    CHECK(PointF(5, 500).ConstrainedWithin(RectF(10, 20, 100, 100)) == PointF(10, 120));
    CHECK(AreEssentiallyEqual(RectF(1, 2, 3, 4), RectF(1, 2, 3, 4.0000001f)));
    CHECK(RectF::NarrowPrecision(1.5, 2.5, 3.5, 4.5) == RectF(1.5f, 2.5f, 3.5f, 4.5f));

    CHECK(RectF(0, 0, 10, 10).IsExpressibleAsRectI());
    CHECK_FALSE(RectF(0, 0, 1e10f, 10).IsExpressibleAsRectI());
    CHECK_FALSE(RectF(2e9f, 0, 2e9f, 10).IsExpressibleAsRectI());

    CHECK(RectF(0, 0, 3, 4).Area() == 12);
    CHECK(RectF(PointF(1, 2), PointF(4, 6)) == RectF(1, 2, 3, 4));
  }
}

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"

namespace krys::boo::tests
{
  using Catch::Matchers::WithinAbs;
  using Catch::Matchers::WithinULP;

  static void TestGetAndSet(FloatRect rect)
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

  static void TestEmptyRect(const FloatRect &rect)
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

  TEST_CASE("FloatRect: DefaultConstruction", "[Graphics][FloatRect]")
  {
    FloatRect test;

    TestEmptyRect(test);

    TestGetAndSet(test);

    auto location = test.Location();
    CHECK_THAT(location.X(), WithinULP(static_cast<float>(0), 4));
    CHECK_THAT(location.Y(), WithinULP(static_cast<float>(0), 4));

    auto size = test.Size();
    CHECK_THAT(size.Width(), WithinULP(static_cast<float>(0), 4));
    CHECK_THAT(size.Height(), WithinULP(static_cast<float>(0), 4));
  }

  TEST_CASE("FloatRect: ValueConstruction", "[Graphics][FloatRect]")
  {
    FloatRect test(10.0f, 20.0f, 100.0f, 50.0f);

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

  TEST_CASE("FloatRect: PointSizeConstruction", "[Graphics][FloatRect]")
  {
    FloatPoint location(20.0f, 30.0f);
    FloatSize size(100.0f, 50.0f);

    FloatRect test(location, size);

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

  TEST_CASE("FloatRect: TwoPointConstruction", "[Graphics][FloatRect]")
  {
    FloatPoint point1(20.0f, 30.0f);
    FloatPoint point2(150.0f, 250.0f);

    FloatRect test(point1, point2);

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

  TEST_CASE("FloatRect: IntRectConstruction", "[Graphics][FloatRect]")
  {
    IntRect rect(20, 30, 150, 300);

    FloatRect test(rect);

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

  TEST_CASE("FloatRect: SetLocationAndSize", "[Graphics][FloatRect]")
  {
    FloatRect rect;

    TestEmptyRect(rect);

    FloatPoint location(10.0f, 20.0f);

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

    FloatSize size(100.0f, 200.0f);

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

  TEST_CASE("FloatRect: Center", "[Graphics][FloatRect]")
  {
    FloatRect rect(20.0f, 30.0f, 100.0f, 200.0f);

    auto center = rect.Center();

    CHECK_THAT(center.X(), WithinULP(70.0f, 4));
    CHECK_THAT(center.Y(), WithinULP(130.0f, 4));
  }

  TEST_CASE("FloatRect: Move", "[Graphics][FloatRect]")
  {
    FloatRect rect(20.0f, 30.0f, 100.0f, 200.0f);

    FloatSize delta(10.0f, 20.0f);

    rect.Move(delta);

    CHECK_THAT(rect.X(), WithinULP(30.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(50.0f, 4));

    FloatPoint deltaPoint(-20.0f, -40.0f);

    rect.MoveBy(deltaPoint);

    CHECK_THAT(rect.X(), WithinULP(10.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(10.0f, 4));

    rect.Move(-10.0f, 22.0f);

    CHECK_THAT(rect.X(), WithinULP(0.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(32.0f, 4));
  }

  TEST_CASE("FloatRect: Expand", "[Graphics][FloatRect]")
  {
    FloatRect rect(20.0f, 30.0f, 100.0f, 200.0f);

    FloatSize size(100.0f, 100.0f);

    rect.Expand(size);

    CHECK_THAT(rect.Width(), WithinULP(200.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(300.0f, 4));

    rect.Expand(55.0f, 22.0f);

    CHECK_THAT(rect.Width(), WithinULP(255.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(322.0f, 4));

    FloatSize size2(-10.0f, -20.0f);

    rect.Expand(size2);

    CHECK_THAT(rect.Width(), WithinULP(245.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(302.0f, 4));

    rect.Expand(-5.0f, -2.0f);

    CHECK_THAT(rect.Width(), WithinULP(240.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(300.0f, 4));

    CHECK_FALSE(rect.IsInfinite());
  }

  TEST_CASE("FloatRect: Contract", "[Graphics][FloatRect]")
  {
    FloatRect rect(20.0f, 30.0f, 100.0f, 200.0f);

    FloatSize size(50.0f, 100.0f);

    rect.Contract(size);

    CHECK_THAT(rect.Width(), WithinULP(50.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(100.0f, 4));

    rect.Contract(25.0f, 22.0f);

    CHECK_THAT(rect.Width(), WithinULP(25.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(78.0f, 4));

    FloatSize size2(-10.0f, -20.0f);

    rect.Contract(size2);

    CHECK_THAT(rect.Width(), WithinULP(35.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(98.0f, 4));

    rect.Contract(-5.0f, -2.0f);

    CHECK_THAT(rect.Width(), WithinULP(40.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(100.0f, 4));

    CHECK_FALSE(rect.IsInfinite());
  }

  TEST_CASE("FloatRect: ShiftXEdge", "[Graphics][FloatRect]")
  {
    FloatRect rect(20.0f, 30.0f, 100.0f, 200.0f);

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

  TEST_CASE("FloatRect: ShiftYEdge", "[Graphics][FloatRect]")
  {
    FloatRect rect(20.0f, 30.0f, 100.0f, 200.0f);

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

  TEST_CASE("FloatRect: Inflate", "[Graphics][FloatRect]")
  {
    FloatRect rect(20.0f, 30.0f, 100.0f, 200.0f);

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

  TEST_CASE("FloatRect: Corners", "[Graphics][FloatRect]")
  {
    FloatRect rect(20.0f, 30.0f, 100.0f, 200.0f);

    FloatPoint topLeft = rect.MinXMinYCorner();
    CHECK_THAT(topLeft.X(), WithinULP(20.0f, 4));
    CHECK_THAT(topLeft.Y(), WithinULP(30.0f, 4));

    FloatPoint topRight = rect.MaxXMinYCorner();
    CHECK_THAT(topRight.X(), WithinULP(120.0f, 4));
    CHECK_THAT(topRight.Y(), WithinULP(30.0f, 4));

    FloatPoint bottomLeft = rect.MinXMaxYCorner();
    CHECK_THAT(bottomLeft.X(), WithinULP(20.0f, 4));
    CHECK_THAT(bottomLeft.Y(), WithinULP(230.0f, 4));

    FloatPoint bottomRight = rect.MaxXMaxYCorner();
    CHECK_THAT(bottomRight.X(), WithinULP(120.0f, 4));
    CHECK_THAT(bottomRight.Y(), WithinULP(230.0f, 4));
  }

  TEST_CASE("FloatRect: Contains", "[Graphics][FloatRect]")
  {
    FloatRect rect(20.0f, 30.0f, 100.0f, 200.0f);

    FloatRect contained(30.0f, 40.0f, 50.0f, 100.0f);

    REQUIRE(rect.Contains(contained));

    FloatRect outside(120.0f, 230.0f, 50.0f, 100.0f);

    REQUIRE_FALSE(rect.Contains(outside));

    FloatRect intersects(10.0f, 20.0f, 90.0f, 180.0f);

    REQUIRE_FALSE(rect.Contains(intersects));

    FloatPoint pointInside(60.0f, 70.0f);

    REQUIRE(rect.Contains(pointInside));
    REQUIRE(rect.Contains(pointInside, FloatRect::ContainsMode::InsideButNotOnStroke));

    FloatPoint pointOutside(160.0f, 270.0f);

    REQUIRE_FALSE(rect.Contains(pointOutside));
    REQUIRE_FALSE(rect.Contains(pointOutside, FloatRect::ContainsMode::InsideButNotOnStroke));

    FloatPoint pointOnLine(20.0f, 30.0f);

    REQUIRE(rect.Contains(pointOnLine));
    REQUIRE_FALSE(rect.Contains(pointOutside, FloatRect::ContainsMode::InsideButNotOnStroke));

    REQUIRE(rect.Contains(60.0f, 70.0f));
    REQUIRE_FALSE(rect.Contains(160.0f, 270.0f));
  }

  TEST_CASE("FloatRect: Intersects", "[Graphics][FloatRect]")
  {
    FloatRect rect(20.0f, 30.0f, 100.0f, 200.0f);

    FloatRect contained(30.0f, 40.0f, 50.0f, 100.0f);

    REQUIRE(rect.Intersects(contained));

    FloatRect outside(120.0f, 230.0f, 50.0f, 100.0f);

    REQUIRE_FALSE(rect.Intersects(outside));

    FloatRect intersects(10.0f, 20.0f, 90.0f, 180.0f);

    REQUIRE(rect.Intersects(intersects));
  }

  static void TestIntersectResult(const FloatRect &rect)
  {
    CHECK_THAT(rect.X(), WithinULP(70.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(120.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(80.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(230.0f, 4));
  }

  TEST_CASE("FloatRect: Intersect", "[Graphics][FloatRect]")
  {
    FloatRect rectA(20.0f, 30.0f, 100.0f, 200.0f);
    FloatRect rectB(70.0f, 80.0f, 100.0f, 200.0f);

    rectA.Intersect(rectB);

    TestIntersectResult(rectA);

    FloatRect rectC(20.0f, 30.0f, 100.0f, 200.0f);

    auto intersected = Intersection(rectC, rectB);

    TestIntersectResult(intersected);
  }

  static void TestUnitedRects(const FloatRect &united)
  {
    CHECK_THAT(united.X(), WithinULP(20.0f, 4));
    CHECK_THAT(united.MaxX(), WithinULP(170.0f, 4));
    CHECK_THAT(united.Y(), WithinULP(30.0f, 4));
    CHECK_THAT(united.MaxY(), WithinULP(280.0f, 4));
  }

  TEST_CASE("FloatRect: Unite", "[Graphics][FloatRect]")
  {
    FloatRect rectA(20.0f, 30.0f, 100.0f, 200.0f);
    FloatRect rectB(70.0f, 80.0f, 100.0f, 200.0f);

    rectA.Unite(rectB);

    TestUnitedRects(rectA);

    FloatRect rectC(20.0f, 30.0f, 100.0f, 200.0f);

    auto united = UnionRect(rectC, rectB);

    TestUnitedRects(united);
  }

  TEST_CASE("FloatRect: Extend", "[Graphics][FloatRect]")
  {
    FloatRect rect(20.0f, 30.0f, 100.0f, 200.0f);
    FloatPoint point(170.0f, 280.0f);

    rect.Extend(point);

    CHECK_THAT(rect.X(), WithinULP(20.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(170.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(30.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(280.0f, 4));
  }

  TEST_CASE("FloatRect: Overlaps", "[Graphics][FloatRect]")
  {
    FloatRect rect(20.0f, 30.0f, 100.0f, 200.0f);

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

  TEST_CASE("FloatRect: Scale", "[Graphics][FloatRect]")
  {
    FloatRect rect(20.0f, 30.0f, 100.0f, 200.0f);

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

  TEST_CASE("FloatRect: Transpose", "[Graphics][FloatRect]")
  {
    FloatRect rect(20.0f, 30.0f, 100.0f, 200.0f);

    auto transposed = rect.TransposedRect();

    CHECK_THAT(transposed.X(), WithinULP(30.0f, 4));
    CHECK_THAT(transposed.MaxX(), WithinULP(230.0f, 4));
    CHECK_THAT(transposed.Y(), WithinULP(20.0f, 4));
    CHECK_THAT(transposed.MaxY(), WithinULP(120.0f, 4));
  }

  static void CheckAdditionResult1(const FloatRect &rect)
  {
    CHECK_THAT(rect.X(), WithinULP(10.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(130.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(20.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(220.0f, 4));
    CHECK_THAT(rect.Width(), WithinULP(120.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(200.0f, 4));
  }

  static void CheckAdditionResult2(const FloatRect &rect)
  {
    CHECK_THAT(rect.X(), WithinULP(10.0f, 4));
    CHECK_THAT(rect.MaxX(), WithinULP(250.0f, 4));
    CHECK_THAT(rect.Y(), WithinULP(20.0f, 4));
    CHECK_THAT(rect.MaxY(), WithinULP(240.0f, 4));
    CHECK_THAT(rect.Width(), WithinULP(240.0f, 4));
    CHECK_THAT(rect.Height(), WithinULP(220.0f, 4));
  }

  TEST_CASE("FloatRect: Addition", "[Graphics][FloatRect]")
  {
    FloatRect rect(10.0f, 20.0f, 100.0f, 100.0f);
    FloatRect rightSide(0.0f, 0.0f, 20.0f, 100.0f);
    FloatRect bottom(0.0f, 0.0f, 120.0f, 20.0f);

    auto combined = rect + rightSide;

    CheckAdditionResult1(combined);

    auto combined2 = combined + bottom;

    CheckAdditionResult2(combined2);

    rect += rightSide;
    rect += bottom;

    CheckAdditionResult2(rect);
  }

  TEST_CASE("FloatRect: Equality", "[Graphics][FloatRect]")
  {
    FloatRect rect(10.0f, 20.0f, 100.0f, 100.0f);
    FloatRect rect2(10.0f, 20.0f, 100.0f, 100.0f);
    FloatRect rightSide(110.0f, 20.0f, 20.0f, 100.0f);

    REQUIRE(rect == rect2);
    REQUIRE_FALSE(rect != rect2);
    REQUIRE(rect != rightSide);
    REQUIRE_FALSE(rect == rightSide);
  }

  TEST_CASE("FloatRect: InfiniteRect", "[Graphics][FloatRect]")
  {
    FloatRect infinite = FloatRect::InfiniteRect();

    CHECK_THAT(infinite.X(), WithinULP(static_cast<float>(-std::numeric_limits<float>::max() / 2), 4));
    CHECK_THAT(infinite.Y(), WithinULP(static_cast<float>(-std::numeric_limits<float>::max() / 2), 4));
    CHECK_THAT(infinite.Width(), WithinULP(static_cast<float>(std::numeric_limits<float>::max()), 4));
    CHECK_THAT(infinite.Height(), WithinULP(static_cast<float>(std::numeric_limits<float>::max()), 4));

    // FIXME: We have an unusual representation for our infinite rect.
    // Float::infiniteRect is (negative infinity)/2 for the upper left corner,
    // while CoreGraphics uses (negative infinity).
  }

  TEST_CASE("FloatRect: EnclosingAndRounding", "[Graphics][FloatRect]")
  {
    FloatRect rect(10.0f, 20.0f, 1024.3f, 768.3f);

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

  TEST_CASE("FloatRect: EnclosingIntRect", "[Graphics][FloatRect]")
  {
    FloatRect rect(10.0f, 20.0f, 1024.3f, 768.6f);

    auto enclosed = EnclosingIntRect(rect);

    CHECK(10 == enclosed.X());
    CHECK(20 == enclosed.Y());
    CHECK(1'035 == enclosed.MaxX());
    CHECK(789 == enclosed.MaxY());

    FloatRect maxIntRect(INT_MIN, INT_MIN, 0, 0);
    maxIntRect.ShiftMaxXEdgeTo(30);
    maxIntRect.ShiftMaxYEdgeTo(30);

    auto enclosed2 = EnclosingIntRect(maxIntRect);

    CHECK(INT_MIN == enclosed2.X());
    CHECK(INT_MIN == enclosed2.Y());
    CHECK(INT_MAX == enclosed2.Width());
    CHECK(INT_MAX == enclosed2.Height());

    FloatRect smallDimensionsRect(42.5f, 84.5f, std::numeric_limits<float>::epsilon(),
                                  std::numeric_limits<float>::epsilon());
    CHECK(IntRect(42, 84, 1, 1) == EnclosingIntRect(smallDimensionsRect));

    FloatRect integralRect(100, 150, 200, 350);
    CHECK(IntRect(100, 150, 200, 350) == EnclosingIntRect(integralRect));

    FloatRect fractionalPosRect(100.6f, 150.8f, 200, 350);
    CHECK(IntRect(100, 150, 201, 351) == EnclosingIntRect(fractionalPosRect));

    FloatRect fractionalDimensionsRect(100, 150, 200.6f, 350.4f);
    CHECK(IntRect(100, 150, 201, 351) == EnclosingIntRect(fractionalDimensionsRect));

    FloatRect fractionalBothRect1(100.6f, 150.8f, 200.4f, 350.2f);
    CHECK(IntRect(100, 150, 201, 351) == EnclosingIntRect(fractionalBothRect1));

    FloatRect fractionalBothRect2(100.6f, 150.8f, 200.3f, 350.3f);
    CHECK(IntRect(100, 150, 201, 352) == EnclosingIntRect(fractionalBothRect2));

    FloatRect fractionalBothRect3(100.6f, 150.8f, 200.5f, 350.3f);
    CHECK(IntRect(100, 150, 202, 352) == EnclosingIntRect(fractionalBothRect3));

    FloatRect fractionalNegposRect1(-100.4f, -150.8f, 200, 350);
    CHECK(IntRect(-101, -151, 201, 351) == EnclosingIntRect(fractionalNegposRect1));

    FloatRect fractionalNegposRect2(-100.4f, -150.8f, 199.4f, 350.3f);
    CHECK(IntRect(-101, -151, 200, 351) == EnclosingIntRect(fractionalNegposRect2));

    FloatRect fractionalNegposRect3(-100.6f, -150.8f, 199.6f, 350.3f);
    CHECK(IntRect(-101, -151, 201, 351) == EnclosingIntRect(fractionalNegposRect3));

    constexpr int intMin = std::numeric_limits<int>::min();
    constexpr int intMax = std::numeric_limits<int>::max();
    FloatRect maxRect(-std::numeric_limits<float>::max() / 2, -std::numeric_limits<float>::max() / 2,
                      std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    CHECK(IntRect(intMin, intMin, intMax, intMax) == EnclosingIntRect(maxRect));
  }

  TEST_CASE("FloatRect: RoundedIntRect", "[Graphics][FloatRect]")
  {
    FloatRect rect(10.0f, 20.0f, 1024.3f, 768.6f);

    auto enclosed = RoundedIntRect(rect);

    CHECK(10 == enclosed.X());
    CHECK(20 == enclosed.Y());
    CHECK(1'034 == enclosed.MaxX());
    CHECK(789 == enclosed.MaxY());
  }

  TEST_CASE("FloatRect: Markable", "[Graphics][FloatRect]")
  {
    FloatRect rect(10.0f, 20.0f, 1024.3f, 768.6f);
    Markable<FloatRect> optional;
    CHECK_FALSE(optional);
    optional = rect;
    CHECK((optional.value_or(FloatRect {})) == rect);
    optional = FloatRect::NanRect();
    CHECK_FALSE(optional);
  }

  TEST_CASE("FloatRect: Intersection variants", "[Graphics][FloatRect]")
  {
    // Touching rects don't intersect, but do inclusively.
    FloatRect a(0, 0, 10, 10);
    FloatRect b(10, 0, 10, 10);
    CHECK_FALSE(a.Intersects(b));
    CHECK(a.InclusivelyIntersects(b));
    CHECK_FALSE(a.InclusivelyIntersects(FloatRect(10.5f, 0, 10, 10)));
    // An empty rect still inclusively intersects.
    CHECK(a.InclusivelyIntersects(FloatRect(5, 5, 0, 0)));
    CHECK_FALSE(a.Intersects(FloatRect(5, 5, 0, 0)));

    FloatRect edge = a;
    CHECK(edge.EdgeInclusiveIntersect(b));
    CHECK(edge == FloatRect(10, 0, 0, 10));

    FloatRect apart = a;
    CHECK_FALSE(apart.EdgeInclusiveIntersect(FloatRect(20, 20, 5, 5)));
    CHECK(apart == FloatRect());

    FloatRect disjoint = a;
    disjoint.Intersect(FloatRect(20, 20, 5, 5));
    CHECK(disjoint == FloatRect());
  }

  TEST_CASE("FloatRect: Union variants", "[Graphics][FloatRect]")
  {
    FloatRect rect(10, 10, 10, 10);
    rect.Unite(FloatRect(100, 100, 0, 5));
    CHECK(rect == FloatRect(10, 10, 10, 10));
    rect.UniteEvenIfEmpty(FloatRect(100, 100, 0, 5));
    CHECK(rect == FloatRect(10, 10, 90, 95));

    FloatRect nonZero(10, 10, 10, 10);
    nonZero.UniteIfNonZero(FloatRect(0, 0, 0, 0));
    CHECK(nonZero == FloatRect(10, 10, 10, 10));
    nonZero.UniteIfNonZero(FloatRect(0, 0, 0, 5));
    CHECK(nonZero == FloatRect(0, 0, 20, 20));

    FloatRect extended(10, 10, 10, 10);
    extended.Extend(FloatPoint(0, 5), FloatPoint(30, 25));
    CHECK(extended == FloatRect(0, 5, 30, 20));
  }

  TEST_CASE("FloatRect: Contains modes", "[Graphics][FloatRect]")
  {
    FloatRect rect(10, 10, 10, 10);
    CHECK(rect.Contains(FloatPoint(10, 15)));
    CHECK(rect.Contains(FloatPoint(10, 15), FloatRect::ContainsMode::InsideOrOnStroke));
    CHECK_FALSE(rect.Contains(FloatPoint(10, 15), FloatRect::ContainsMode::InsideButNotOnStroke));
    CHECK(rect.Contains(FloatPoint(15, 15), FloatRect::ContainsMode::InsideButNotOnStroke));
  }

  TEST_CASE("FloatRect: Box extents and edges", "[Graphics][FloatRect]")
  {
    FloatRect rect(10, 10, 10, 10);
    FloatBoxExtent box(1.0f, 2.0f, 3.0f, 4.0f);
    rect.Expand(box);
    CHECK(rect == FloatRect(6, 9, 16, 14));
    CHECK(FloatRect(10, 10, 10, 10) + box == rect);
    rect.Contract(box);
    CHECK(rect == FloatRect(10, 10, 10, 10));

    rect.ShiftEdgesTo(1, 2, 5, 10);
    CHECK(rect == FloatRect(1, 2, 4, 8));
    rect.ShiftXEdgeBy(1);
    rect.ShiftYEdgeBy(1);
    rect.ShiftMaxXEdgeBy(2);
    rect.ShiftMaxYEdgeBy(-10);
    CHECK(rect == FloatRect(2, 3, 5, 0));

    rect.Inflate(1, 2, 3, 4);
    CHECK(rect == FloatRect(1, 1, 9, 6));
    rect.Inflate(FloatSize(1, 1));
    CHECK(rect == FloatRect(0, 0, 11, 8));
  }

  TEST_CASE("FloatRect: Special rects and helpers", "[Graphics][FloatRect]")
  {
    CHECK(FloatRect::InfiniteRect().IsInfinite());
    CHECK(FloatRect::SmallestRect().IsSmallest());
    CHECK_FALSE(FloatRect().IsSmallest());
    CHECK(FloatRect::NanRect().IsNaN());
    CHECK_FALSE(FloatRect().IsNaN());

    CHECK(NormaliseRect(FloatRect(10, 20, -5, -10)) == FloatRect(5, 10, 5, 10));
    CHECK(NormaliseRect(FloatRect(10, 20, 5, 10)) == FloatRect(10, 20, 5, 10));

    CHECK(EnclosingIntRectPreservingEmptyRects(FloatRect(1.5f, 2.5f, 0, 3)) == IntRect(1, 2, 0, 0));
    CHECK(EnclosingIntRectPreservingEmptyRects(FloatRect(1.5f, 2.5f, 1, 3)) == IntRect(1, 2, 2, 4));

    CHECK(FloatPoint(5, 500).ConstrainedWithin(FloatRect(10, 20, 100, 100)) == FloatPoint(10, 120));
    CHECK(AreEssentiallyEqual(FloatRect(1, 2, 3, 4), FloatRect(1, 2, 3, 4.0000001f)));
    CHECK(FloatRect::NarrowPrecision(1.5, 2.5, 3.5, 4.5) == FloatRect(1.5f, 2.5f, 3.5f, 4.5f));

    CHECK(FloatRect(0, 0, 10, 10).IsExpressibleAsIntRect());
    CHECK_FALSE(FloatRect(0, 0, 1e10f, 10).IsExpressibleAsIntRect());
    CHECK_FALSE(FloatRect(2e9f, 0, 2e9f, 10).IsExpressibleAsIntRect());

    CHECK(FloatRect(0, 0, 3, 4).Area() == 12);
    CHECK(FloatRect(FloatPoint(1, 2), FloatPoint(4, 6)) == FloatRect(1, 2, 3, 4));
  }
}

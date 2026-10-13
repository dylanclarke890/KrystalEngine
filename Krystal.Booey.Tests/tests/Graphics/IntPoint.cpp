#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"

namespace krys::boo::tests
{
  namespace
  {
    void TestGetAndSet(IntPoint point)
    {
      point.SetX(1);
      CHECK(1 == point.X());
      point.SetY(2);
      CHECK(2 == point.Y());
    }
  }

  TEST_CASE("IntPoint: construction", "[Graphics][IntPoint]")
  {
    constexpr IntPoint defaultConstructed;
    STATIC_CHECK(defaultConstructed.X() == 0);
    STATIC_CHECK(defaultConstructed.Y() == 0);
    STATIC_CHECK(defaultConstructed.IsZero());
    TestGetAndSet(defaultConstructed);

    constexpr IntPoint value(10, 9);
    STATIC_CHECK(value.X() == 10);
    STATIC_CHECK(value.Y() == 9);
    TestGetAndSet(value);

    STATIC_CHECK(IntPoint::Zero().IsZero());

    constexpr IntPoint fromSize(IntSize(2'003, 1'997));
    STATIC_CHECK(fromSize.X() == 2'003);
    STATIC_CHECK(fromSize.Y() == 1'997);
    STATIC_CHECK_FALSE(fromSize.IsZero());

    IntPoint fromFloat(FloatPoint(2003.2f, 1997.3f));
    CHECK(fromFloat.X() == 2'003);
    CHECK(fromFloat.Y() == 1'997);
    CHECK_FALSE(fromFloat.IsZero());

    IntPoint clamped(FloatPoint(1e20f, -1e20f));
    CHECK(clamped.X() == Limits<int32>::max());
    CHECK(clamped.Y() == Limits<int32>::min());
  }

  TEST_CASE("IntPoint: move and scale", "[Graphics][IntPoint]")
  {
    IntPoint test(10, 20);
    test.Move(IntSize(30, 50));
    CHECK(test == IntPoint(40, 70));
    test.Move(-2, 8);
    CHECK(test == IntPoint(38, 78));
    test.MoveBy(IntPoint(100, 100));
    CHECK(test == IntPoint(138, 178));

    IntPoint scaled(10, 20);
    scaled.Scale(2.0f);
    CHECK(scaled == IntPoint(20, 40));
    scaled.Scale(3.0f, 1.5f);
    CHECK(scaled == IntPoint(60, 60));

    CHECK(IntPoint(10, 20).Scaled(1.25f) == IntPoint(13, 25));
    CHECK(IntPoint(10, 20).Scaled(0.5f, 2.0f) == IntPoint(5, 40));
  }

  TEST_CASE("IntPoint: expand, shrink, constrain and transpose", "[Graphics][IntPoint]")
  {
    constexpr IntPoint a(10, 20);
    constexpr IntPoint b(20, 40);
    STATIC_CHECK(a.ExpandedTo(b) == IntPoint(20, 40));
    STATIC_CHECK(b.ShrunkTo(a) == IntPoint(10, 20));
    STATIC_CHECK(a.TransposedPoint() == IntPoint(20, 10));
    STATIC_CHECK(IntPoint(0, 50).ConstrainedBetween(a, b) == IntPoint(10, 40));
    STATIC_CHECK(a.DistanceSquaredToPoint(b) == 500);

    IntPoint negative(-5, 5);
    negative.ClampNegativeToZero();
    CHECK(negative == IntPoint(0, 5));
  }

  TEST_CASE("IntPoint: arithmetic", "[Graphics][IntPoint]")
  {
    STATIC_CHECK(ToIntSize(IntPoint(10, 20)) == IntSize(10, 20));

    IntPoint a(10, 20);
    STATIC_CHECK(IntPoint(10, 20) + IntPoint(50, 60) == IntPoint(60, 80));
    STATIC_CHECK(IntPoint(10, 20) + IntSize(50, 60) == IntPoint(60, 80));
    a += IntSize(50, 60);
    CHECK(a == IntPoint(60, 80));

    IntPoint b(100, 80);
    STATIC_CHECK(IntPoint(100, 80) - IntPoint(50, 60) == IntSize(50, 20));
    STATIC_CHECK(IntPoint(100, 80) - IntSize(50, 60) == IntPoint(50, 20));
    b -= IntSize(50, 60);
    CHECK(b == IntPoint(50, 20));

    STATIC_CHECK(-IntPoint(100, 80) == IntPoint(-100, -80));

    STATIC_CHECK(IntPoint(100, 80) == IntPoint(100, 80));
    STATIC_CHECK(IntPoint(100, 80) != IntPoint(70, 50));
  }
}

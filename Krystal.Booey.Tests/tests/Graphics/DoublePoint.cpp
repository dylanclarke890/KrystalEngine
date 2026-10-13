#include "Krystal.Booey/Graphics/DoublePoint.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutPoint.hpp"

namespace krys::boo::tests
{
  TEST_CASE("DoubleSize", "[Graphics][DoubleSize]")
  {
    STATIC_CHECK(DoubleSize() == DoubleSize(0, 0));
    STATIC_CHECK(DoubleSize(2, 3).Width() == 2);
    STATIC_CHECK(DoubleSize(2, 3).Height() == 3);
    STATIC_CHECK(DoubleSize(2, 3).ScaledBy(2) == DoubleSize(4, 6));
    STATIC_CHECK(DoubleSize(2, 3).ScaledBy(2, 0.5) == DoubleSize(4, 1.5));

    CHECK(DoubleSize(FloatSize(1.5f, -2.5f)) == DoubleSize(1.5, -2.5));
    DoubleSize converted = FloatSize(1.5f, 2.5f);
    CHECK(converted == DoubleSize(1.5, 2.5));
  }

  TEST_CASE("DoublePoint: construction", "[Graphics][DoublePoint]")
  {
    STATIC_CHECK(DoublePoint().IsZero());
    STATIC_CHECK(DoublePoint::Zero() == DoublePoint(0, 0));
    STATIC_CHECK_FALSE(DoublePoint(0, 1).IsZero());
    STATIC_CHECK(DoublePoint(IntPoint(3, -4)) == DoublePoint(3, -4));
    STATIC_CHECK(DoublePoint(DoubleSize(1.25, 2.5)) == DoublePoint(1.25, 2.5));

    // Conversions to and from the float and layout points.
    CHECK(FloatPoint(DoublePoint(1.5, 2.25)) == FloatPoint(1.5f, 2.25f));
    DoublePoint fromFloat = FloatPoint(1.5f, 2.25f);
    CHECK(fromFloat == DoublePoint(1.5, 2.25));
    CHECK(LayoutPoint(DoublePoint(1.5, 2.25)) == LayoutPoint(LayoutUnit(1.5), LayoutUnit(2.25)));
    DoublePoint fromLayout = LayoutPoint(LayoutUnit(1.5), LayoutUnit(-2.25));
    CHECK(fromLayout == DoublePoint(1.5, -2.25));
  }

  TEST_CASE("DoublePoint: move and scale", "[Graphics][DoublePoint]")
  {
    DoublePoint p(1, 2);
    p.Move(0.5, 0.25);
    CHECK(p == DoublePoint(1.5, 2.25));
    p.Move(IntSize(1, 1));
    CHECK(p == DoublePoint(2.5, 3.25));
    p.Move(DoubleSize(-0.5, -0.25));
    CHECK(p == DoublePoint(2, 3));
    p.MoveBy(DoublePoint(1, 1));
    CHECK(p == DoublePoint(3, 4));
    p.Scale(2);
    CHECK(p == DoublePoint(6, 8));
    CHECK(p.Scaled(0.5) == DoublePoint(3, 4));
  }

  TEST_CASE("DoublePoint: arithmetic and rounding", "[Graphics][DoublePoint]")
  {
    STATIC_CHECK(DoublePoint(1, 2) + DoubleSize(3, 4) == DoublePoint(4, 6));
    STATIC_CHECK(DoublePoint(1, 2) + DoublePoint(3, 4) == DoublePoint(4, 6));
    STATIC_CHECK(DoublePoint(1, 2) - DoublePoint(3, 5) == DoubleSize(-2, -3));
    STATIC_CHECK(DoublePoint(1, 2) - DoubleSize(3, 5) == DoublePoint(-2, -3));
    STATIC_CHECK(ToDoubleSize(DoublePoint(1, 2)) == DoubleSize(1, 2));

    CHECK(FlooredIntPoint(DoublePoint(1.7, -1.2)) == IntPoint(1, -2));
    CHECK(RoundedIntPoint(DoublePoint(1.5, -1.2)) == IntPoint(2, -1));
    CHECK(RoundedIntPoint(DoublePoint(1e20, -1e20)) == IntPoint(Limits<int>::max(), Limits<int>::min()));
  }
}

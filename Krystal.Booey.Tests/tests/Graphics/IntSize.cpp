#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"

namespace krys::boo::tests
{
  TEST_CASE("IntSize", "[Graphics][IntSize]")
  {
    constexpr IntSize size(4, 2);
    STATIC_CHECK(size.Width() == 4);
    STATIC_CHECK(size.Height() == 2);
    STATIC_CHECK(size.MinDimension() == 2);
    STATIC_CHECK(size.MaxDimension() == 4);
    STATIC_CHECK(size.AspectRatio() == 2.0f);
    STATIC_CHECK(size.DiagonalLengthSquared() == 20);
    STATIC_CHECK(size.TransposedSize() == IntSize(2, 4));
    STATIC_CHECK(size.Scaled(1.5f) == IntSize(6, 3));
    STATIC_CHECK(size.Scaled(0.5f, 2.0f) == IntSize(2, 4));
    STATIC_CHECK(size.ExpandedTo({3, 5}) == IntSize(4, 5));
    STATIC_CHECK(size.ShrunkTo({3, 5}) == IntSize(3, 2));
    STATIC_CHECK(size.ConstrainedBetween({5, 0}, {6, 1}) == IntSize(5, 1));
    STATIC_CHECK(IntSize(-3, 4).UnclampedArea() == 12);
    STATIC_CHECK(IntSize(Limits<int32>::min(), 2).UnclampedArea() == 4'294'967'296ull);

    STATIC_CHECK(IntSize().IsEmpty());
    STATIC_CHECK(IntSize().IsZero());
    STATIC_CHECK(IntSize(1, 0).IsEmpty());
    STATIC_CHECK_FALSE(IntSize(1, 0).IsZero());
    STATIC_CHECK_FALSE(size.IsEmpty());

    STATIC_CHECK(size + IntSize(1, 1) == IntSize(5, 3));
    STATIC_CHECK(size - IntSize(1, 1) == IntSize(3, 1));
    STATIC_CHECK(-size == IntSize(-4, -2));

    auto mutableSize = size;
    mutableSize += IntSize(1, 1);
    CHECK(mutableSize == IntSize(5, 3));
    mutableSize -= IntSize(2, 2);
    CHECK(mutableSize == IntSize(3, 1));
    mutableSize.Expand(1, 2);
    CHECK(mutableSize == IntSize(4, 3));
    mutableSize.Contract(1, 1);
    CHECK(mutableSize == IntSize(3, 2));
    mutableSize.Scale(2.0f);
    CHECK(mutableSize == IntSize(6, 4));
    mutableSize.Scale(0.5f, 0.25f);
    CHECK(mutableSize == IntSize(3, 1));
    mutableSize.ClampToMinimumSize({4, 0});
    CHECK(mutableSize == IntSize(4, 1));
    mutableSize.ClampToMaximumSize({2, 2});
    CHECK(mutableSize == IntSize(2, 1));
    mutableSize = IntSize(-1, 3);
    mutableSize.ClampNegativeToZero();
    CHECK(mutableSize == IntSize(0, 3));
  }

  TEST_CASE("IntSize: FloatSize conversions", "[Graphics][IntSize][FloatSize]")
  {
    CHECK(IntSize(FloatSize(2.7f, -2.7f)) == IntSize(2, -2));
    CHECK(IntSize(FloatSize(1e20f, -1e20f)) == IntSize(Limits<int32>::max(), Limits<int32>::min()));
    STATIC_CHECK(FloatSize(IntSize(3, 4)) == FloatSize(3.0f, 4.0f));

    CHECK(RoundedIntSize(FloatSize(1.5f, -1.4f)) == IntSize(2, -1));
    CHECK(FlooredIntSize(FloatSize(1.5f, -1.4f)) == IntSize(1, -2));
    CHECK(ExpandedIntSize(FloatSize(1.5f, -1.4f)) == IntSize(2, -1));
    CHECK(FlooredIntPoint(FloatSize(1.5f, -1.4f)) == IntPoint(1, -2));

    STATIC_CHECK(FloatSize(1e9f, -1e9f).IsExpressibleAsIntSize());
    STATIC_CHECK_FALSE(FloatSize(1e10f, 0.0f).IsExpressibleAsIntSize());
  }
}

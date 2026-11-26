#include "Krystal.Core/Geometry/Size.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Geometry/Point.hpp"

namespace krys::tests
{
  TEST_CASE("SizeI", "[Geometry][SizeI]")
  {
    constexpr SizeI size(4, 2);
    STATIC_CHECK(size.Width() == 4);
    STATIC_CHECK(size.Height() == 2);
    STATIC_CHECK(size.MinDimension() == 2);
    STATIC_CHECK(size.MaxDimension() == 4);
    STATIC_CHECK(size.AspectRatio() == 2.0f);
    STATIC_CHECK(size.DiagonalLengthSquared() == 20);
    STATIC_CHECK(size.TransposedSize() == SizeI(2, 4));
    STATIC_CHECK(size.Scaled(1.5f) == SizeI(6, 3));
    STATIC_CHECK(size.Scaled(0.5f, 2.0f) == SizeI(2, 4));
    STATIC_CHECK(size.ExpandedTo({3, 5}) == SizeI(4, 5));
    STATIC_CHECK(size.ShrunkTo({3, 5}) == SizeI(3, 2));
    STATIC_CHECK(size.ConstrainedBetween({5, 0}, {6, 1}) == SizeI(5, 1));
    STATIC_CHECK(SizeI(-3, 4).UnclampedArea() == 12);
    STATIC_CHECK(SizeI(Limits<int32>::min(), 2).UnclampedArea() == 4'294'967'296ull);

    STATIC_CHECK(SizeI().IsEmpty());
    STATIC_CHECK(SizeI().IsZero());
    STATIC_CHECK(SizeI(1, 0).IsEmpty());
    STATIC_CHECK_FALSE(SizeI(1, 0).IsZero());
    STATIC_CHECK_FALSE(size.IsEmpty());

    STATIC_CHECK(size + SizeI(1, 1) == SizeI(5, 3));
    STATIC_CHECK(size - SizeI(1, 1) == SizeI(3, 1));
    STATIC_CHECK(-size == SizeI(-4, -2));

    auto mutableSize = size;
    mutableSize += SizeI(1, 1);
    CHECK(mutableSize == SizeI(5, 3));
    mutableSize -= SizeI(2, 2);
    CHECK(mutableSize == SizeI(3, 1));
    mutableSize.Expand(1, 2);
    CHECK(mutableSize == SizeI(4, 3));
    mutableSize.Contract(1, 1);
    CHECK(mutableSize == SizeI(3, 2));
    mutableSize.Scale(2.0f);
    CHECK(mutableSize == SizeI(6, 4));
    mutableSize.Scale(0.5f, 0.25f);
    CHECK(mutableSize == SizeI(3, 1));
    mutableSize.ClampToMinimumSize({4, 0});
    CHECK(mutableSize == SizeI(4, 1));
    mutableSize.ClampToMaximumSize({2, 2});
    CHECK(mutableSize == SizeI(2, 1));
    mutableSize = SizeI(-1, 3);
    mutableSize.ClampNegativeToZero();
    CHECK(mutableSize == SizeI(0, 3));
  }

  TEST_CASE("SizeI: SizeF conversions", "[Graphics][SizeI][SizeF]")
  {
    CHECK(SizeI(SizeF(2.7f, -2.7f)) == SizeI(2, -2));
    CHECK(SizeI(SizeF(1e20f, -1e20f)) == SizeI(Limits<int32>::max(), Limits<int32>::min()));
    STATIC_CHECK(SizeF(SizeI(3, 4)) == SizeF(3.0f, 4.0f));

    CHECK(RoundedSizeI(SizeF(1.5f, -1.4f)) == SizeI(2, -1));
    CHECK(FlooredSizeI(SizeF(1.5f, -1.4f)) == SizeI(1, -2));
    CHECK(ExpandedSizeI(SizeF(1.5f, -1.4f)) == SizeI(2, -1));
    CHECK(FlooredPointI(SizeF(1.5f, -1.4f)) == PointI(1, -2));

    STATIC_CHECK(SizeF(1e9f, -1e9f).IsExpressibleAsSizeI());
    STATIC_CHECK_FALSE(SizeF(1e10f, 0.0f).IsExpressibleAsSizeI());
  }

  TEST_CASE("SizeF", "[Geometry][SizeF]")
  {
    constexpr auto size = SizeF {4.0f, 2.0f};
    STATIC_CHECK(size.Width() == 4.0f);
    STATIC_CHECK(size.Height() == 2.0f);
    STATIC_CHECK(size.MinDimension() == 2.0f);
    STATIC_CHECK(size.MaxDimension() == 4.0f);
    STATIC_CHECK(size.AspectRatio() == 2.0f);
    STATIC_CHECK(size.Area() == 8.0f);
    STATIC_CHECK(size.DiagonalLengthSquared() == 20.0f);
    STATIC_CHECK(size.TransposedSize() == SizeF {2.0f, 4.0f});
    STATIC_CHECK(size.Scaled(2.0f) == SizeF {8.0f, 4.0f});
    STATIC_CHECK(size.ScaledBy(2.0f, 0.5f) == SizeF {8.0f, 1.0f});
    STATIC_CHECK(size.ExpandedTo({3.0f, 5.0f}) == SizeF {4.0f, 5.0f});
    STATIC_CHECK(size.ShrunkTo({3.0f, 5.0f}) == SizeF {3.0f, 2.0f});
    STATIC_CHECK(size.ConstrainedBetween({5.0f, 0.0f}, {6.0f, 1.0f}) == SizeF {5.0f, 1.0f});

    STATIC_CHECK(SizeF {}.IsEmpty());
    STATIC_CHECK(SizeF {}.IsZero());
    STATIC_CHECK(SizeF {1.0f, 0.0f}.IsEmpty());
    STATIC_CHECK_FALSE(size.IsEmpty());
    STATIC_CHECK_FALSE(size.IsZero());
    CHECK(SizeF::NanSize().IsNaN());
    CHECK_FALSE(size.IsNaN());

    STATIC_CHECK(size + SizeF {1.0f, 1.0f} == SizeF {5.0f, 3.0f});
    STATIC_CHECK(size - SizeF {1.0f, 1.0f} == SizeF {3.0f, 1.0f});
    STATIC_CHECK(-size == SizeF {-4.0f, -2.0f});
    STATIC_CHECK(size * 0.5f == SizeF {2.0f, 1.0f});
    STATIC_CHECK(size / 2.0f == SizeF {2.0f, 1.0f});
    STATIC_CHECK(8.0f / size == SizeF {2.0f, 4.0f});
    STATIC_CHECK(size * size == SizeF {16.0f, 4.0f});

    auto mutableSize = size;
    mutableSize += SizeF {1.0f, 1.0f};
    mutableSize.Expand(1.0f, 2.0f);
    mutableSize.Scale(2.0f);
    CHECK(mutableSize == SizeF {12.0f, 10.0f});
    CHECK(size.DiagonalLength() == std::hypot(4.0f, 2.0f));
    CHECK(AreEssentiallyEqual(SizeF {1.0f, 2.0f}, SizeF {1.0f, 2.0f + 1e-7f}));
    CHECK(SizeF::NarrowPrecision(1.5, 2.5) == SizeF {1.5f, 2.5f});
  }
}

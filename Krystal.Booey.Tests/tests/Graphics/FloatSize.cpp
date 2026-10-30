#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  TEST_CASE("FloatSize", "[Graphics][FloatSize]")
  {
    constexpr auto size = FloatSize {4.0f, 2.0f};
    STATIC_CHECK(size.Width() == 4.0f);
    STATIC_CHECK(size.Height() == 2.0f);
    STATIC_CHECK(size.MinDimension() == 2.0f);
    STATIC_CHECK(size.MaxDimension() == 4.0f);
    STATIC_CHECK(size.AspectRatio() == 2.0f);
    STATIC_CHECK(size.Area() == 8.0f);
    STATIC_CHECK(size.DiagonalLengthSquared() == 20.0f);
    STATIC_CHECK(size.TransposedSize() == FloatSize {2.0f, 4.0f});
    STATIC_CHECK(size.Scaled(2.0f) == FloatSize {8.0f, 4.0f});
    STATIC_CHECK(size.ScaledBy(2.0f, 0.5f) == FloatSize {8.0f, 1.0f});
    STATIC_CHECK(size.ExpandedTo({3.0f, 5.0f}) == FloatSize {4.0f, 5.0f});
    STATIC_CHECK(size.ShrunkTo({3.0f, 5.0f}) == FloatSize {3.0f, 2.0f});
    STATIC_CHECK(size.ConstrainedBetween({5.0f, 0.0f}, {6.0f, 1.0f}) == FloatSize {5.0f, 1.0f});

    STATIC_CHECK(FloatSize {}.IsEmpty());
    STATIC_CHECK(FloatSize {}.IsZero());
    STATIC_CHECK(FloatSize {1.0f, 0.0f}.IsEmpty());
    STATIC_CHECK_FALSE(size.IsEmpty());
    STATIC_CHECK_FALSE(size.IsZero());
    CHECK(FloatSize::NanSize().IsNaN());
    CHECK_FALSE(size.IsNaN());

    STATIC_CHECK(size + FloatSize {1.0f, 1.0f} == FloatSize {5.0f, 3.0f});
    STATIC_CHECK(size - FloatSize {1.0f, 1.0f} == FloatSize {3.0f, 1.0f});
    STATIC_CHECK(-size == FloatSize {-4.0f, -2.0f});
    STATIC_CHECK(size * 0.5f == FloatSize {2.0f, 1.0f});
    STATIC_CHECK(size / 2.0f == FloatSize {2.0f, 1.0f});
    STATIC_CHECK(8.0f / size == FloatSize {2.0f, 4.0f});
    STATIC_CHECK(size * size == FloatSize {16.0f, 4.0f});

    auto mutableSize = size;
    mutableSize += FloatSize {1.0f, 1.0f};
    mutableSize.Expand(1.0f, 2.0f);
    mutableSize.Scale(2.0f);
    CHECK(mutableSize == FloatSize {12.0f, 10.0f});
    CHECK(size.DiagonalLength() == std::hypot(4.0f, 2.0f));
    CHECK(AreEssentiallyEqual(FloatSize {1.0f, 2.0f}, FloatSize {1.0f, 2.0f + 1e-7f}));
    CHECK(FloatSize::NarrowPrecision(1.5, 2.5) == FloatSize {1.5f, 2.5f});
  }
}

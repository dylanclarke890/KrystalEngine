#include "Krystal.Web/Graphics/Geometry/LayoutSize.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::web::tests
{
  TEST_CASE("LayoutSize", "[Graphics][LayoutSize]")
  {
    constexpr LayoutSize size(4, LayoutUnit(2.5f));
    STATIC_CHECK(size.Width() == LayoutUnit(4));
    STATIC_CHECK(size.Height() == LayoutUnit(2.5));
    STATIC_CHECK(size.MinDimension() == LayoutUnit(2.5));
    STATIC_CHECK(size.MaxDimension() == LayoutUnit(4));
    STATIC_CHECK(size.AspectRatio() == 1.6f);
    STATIC_CHECK(size.TransposedSize() == LayoutSize(LayoutUnit(2.5f), 4));
    STATIC_CHECK(size.ExpandedTo({3, 5}) == LayoutSize(4, 5));
    STATIC_CHECK(size.ShrunkTo({3, 5}) == LayoutSize(3, LayoutUnit(2.5f)));
    STATIC_CHECK(size.ConstrainedBetween({5, 0}, {6, 1}) == LayoutSize(5, 1));
    STATIC_CHECK(LayoutSize(SizeI(3, 4)) == LayoutSize(3, 4));
    STATIC_CHECK(LayoutSize(SizeF(3.5f, 4.0f)) == LayoutSize(LayoutUnit(3.5f), 4));
    STATIC_CHECK(static_cast<SizeF>(size) == SizeF(4.0f, 2.5f));

    STATIC_CHECK(LayoutSize().IsEmpty());
    STATIC_CHECK(LayoutSize().IsZero());
    STATIC_CHECK(LayoutSize(1, 0).IsEmpty());
    STATIC_CHECK_FALSE(size.IsEmpty());
    STATIC_CHECK(MightBeSaturated(LayoutSize(LayoutUnit::Max(), 1)));
    STATIC_CHECK_FALSE(MightBeSaturated(size));

    STATIC_CHECK(size + LayoutSize(1, 1) == LayoutSize(5, LayoutUnit(3.5f)));
    STATIC_CHECK(size - LayoutSize(1, 1) == LayoutSize(3, LayoutUnit(1.5f)));
    STATIC_CHECK(-size == LayoutSize(-4, LayoutUnit(-2.5f)));

    auto mutableSize = size;
    mutableSize += LayoutSize(1, 1);
    CHECK(mutableSize == LayoutSize(5, LayoutUnit(3.5f)));
    mutableSize -= LayoutSize(1, 1);
    CHECK(mutableSize == size);
    mutableSize.Expand(1, LayoutUnit(0.5f));
    CHECK(mutableSize == LayoutSize(5, 3));
    mutableSize.Contract(LayoutUnit(1), LayoutUnit(1));
    CHECK(mutableSize == LayoutSize(4, 2));
    mutableSize.Scale(2.0f);
    CHECK(mutableSize == LayoutSize(8, 4));
    mutableSize.Scale(0.5f, 0.25f);
    CHECK(mutableSize == LayoutSize(4, 1));
    mutableSize.ClampToMinimumSize({5, 0});
    CHECK(mutableSize == LayoutSize(5, 1));
    mutableSize = LayoutSize(-1, 3);
    mutableSize.ClampNegativeToZero();
    CHECK(mutableSize == LayoutSize(0, 3));
    mutableSize.SetWidth(LayoutUnit(2.5f));
    mutableSize.SetHeight(LayoutUnit(7));
    CHECK(mutableSize == LayoutSize(LayoutUnit(2.5f), 7));
  }

  TEST_CASE("LayoutSize: aspect ratio, rounding and limits", "[Graphics][LayoutSize]")
  {
    constexpr LayoutSize ratio(2, 1);
    STATIC_CHECK(FitToAspectRatio(LayoutSize(100, 100), ratio, AspectRatioFit::Shrink)
                 == LayoutSize(100, 50));
    STATIC_CHECK(FitToAspectRatio(LayoutSize(100, 100), ratio, AspectRatioFit::Grow) == LayoutSize(200, 100));

    constexpr LayoutSize size(LayoutUnit(1.5f), LayoutUnit(-1.25f));
    STATIC_CHECK(FlooredSizeI(size) == SizeI(1, -2));
    STATIC_CHECK(CeiledSizeI(size) == SizeI(2, -1));
    STATIC_CHECK(RoundedSizeI(size) == SizeI(2, -1));
    CHECK(FloorSizeToDevicePixels(LayoutSize(LayoutUnit(1.75f), LayoutUnit(1.25f)), 2.0f)
          == SizeF(1.5f, 1.0f));
    CHECK(RoundSizeToDevicePixels(LayoutSize(LayoutUnit(1.75f), LayoutUnit(1.25f)), 2.0f)
          == SizeF(2.0f, 1.5f));

    constexpr LayoutSizeLimits limits {LayoutSize(0, 0), LayoutSize(10, 10)};
    STATIC_CHECK(limits.Clamp(LayoutSize(20, -5)) == LayoutSize(10, 0));
    STATIC_CHECK(limits.Fits(LayoutSize(5, 5)));
    STATIC_CHECK_FALSE(limits.Fits(LayoutSize(20, 5)));
    STATIC_CHECK(limits.Distance(LayoutSize(20, -5)) == LayoutSize(10, -5));
  }
}

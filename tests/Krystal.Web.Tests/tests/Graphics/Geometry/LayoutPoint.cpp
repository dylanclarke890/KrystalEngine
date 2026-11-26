#include "Krystal.Web/Graphics/Geometry/LayoutPoint.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web/Graphics/Animation/AnimationUtilities.hpp"

namespace krys::web::tests
{
  TEST_CASE("LayoutPoint", "[Graphics][LayoutPoint]")
  {
    constexpr LayoutPoint point(LayoutUnit(1.5f), 2);
    STATIC_CHECK(point.X() == LayoutUnit(1.5));
    STATIC_CHECK(point.Y() == LayoutUnit(2));
    STATIC_CHECK(LayoutPoint::Zero().IsZero());
    STATIC_CHECK_FALSE(point.IsZero());
    STATIC_CHECK(LayoutPoint(PointI(1, 2)) == LayoutPoint(1, 2));
    STATIC_CHECK(LayoutPoint(PointF(1.5f, 2.0f)) == point);
    STATIC_CHECK(LayoutPoint(LayoutSize(LayoutUnit(1.5f), 2)) == point);
    STATIC_CHECK(static_cast<PointF>(point) == PointF(1.5f, 2.0f));

    STATIC_CHECK(point.Scaled(2.0f) == LayoutPoint(3, 4));
    STATIC_CHECK(point.Scaled(2.0f, 0.5f) == LayoutPoint(3, 1));
    STATIC_CHECK(point.ExpandedTo({2, 1}) == LayoutPoint(2, 2));
    STATIC_CHECK(point.ShrunkTo({2, 1}) == LayoutPoint(LayoutUnit(1.5f), 1));
    STATIC_CHECK(point.ConstrainedBetween({2, 0}, {3, 1}) == LayoutPoint(2, 1));
    STATIC_CHECK(point.TransposedPoint() == LayoutPoint(2, LayoutUnit(1.5f)));
    STATIC_CHECK(Fraction(LayoutPoint(LayoutUnit(1.25f), LayoutUnit(-1.25f)))
                 == LayoutPoint(LayoutUnit(0.25f), LayoutUnit(-0.25f)));

    LayoutPoint mutablePoint = point;
    mutablePoint.Move(LayoutSize(1, 1));
    CHECK(mutablePoint == LayoutPoint(LayoutUnit(2.5f), 3));
    mutablePoint.MoveBy(LayoutPoint(-1, -1));
    CHECK(mutablePoint == point);
    mutablePoint.Move(LayoutUnit(0.5f), 1);
    CHECK(mutablePoint == LayoutPoint(2, 3));
    mutablePoint.Scale(2.0f);
    CHECK(mutablePoint == LayoutPoint(4, 6));
    mutablePoint.Scale(0.5f, 0.25f);
    CHECK(mutablePoint == LayoutPoint(2, LayoutUnit(1.5f)));
    mutablePoint.SetX(-1);
    mutablePoint.SetY(LayoutUnit(3));
    mutablePoint.ClampNegativeToZero();
    CHECK(mutablePoint == LayoutPoint(0, 3));
  }

  TEST_CASE("LayoutPoint: arithmetic and conversions", "[Graphics][LayoutPoint]")
  {
    STATIC_CHECK(LayoutPoint(1, 2) + LayoutSize(1, 1) == LayoutPoint(2, 3));
    STATIC_CHECK(LayoutPoint(1, 2) + LayoutPoint(1, 1) == LayoutPoint(2, 3));
    STATIC_CHECK(LayoutPoint(1, 2) - LayoutPoint(1, 1) == LayoutSize(0, 1));
    STATIC_CHECK(LayoutPoint(1, 2) - LayoutSize(1, 1) == LayoutPoint(0, 1));
    STATIC_CHECK(-LayoutPoint(1, 2) == LayoutPoint(-1, -2));

    LayoutPoint point(1, 2);
    point += LayoutSize(1, 1);
    CHECK(point == LayoutPoint(2, 3));
    point -= LayoutSize(2, 2);
    CHECK(point == LayoutPoint(0, 1));

    STATIC_CHECK(ToLayoutPoint(LayoutSize(1, 2)) == LayoutPoint(1, 2));
    STATIC_CHECK(ToLayoutSize(LayoutPoint(1, 2)) == LayoutSize(1, 2));

    constexpr LayoutPoint fractional(LayoutUnit(1.5f), LayoutUnit(-1.25f));
    STATIC_CHECK(FlooredPointI(fractional) == PointI(1, -2));
    STATIC_CHECK(RoundedPointI(fractional) == PointI(2, -1));
    STATIC_CHECK(RoundedPointI(LayoutSize(LayoutUnit(1.5f), LayoutUnit(-1.25f))) == PointI(2, -1));
    STATIC_CHECK(CeiledPointI(fractional) == PointI(2, -1));
    CHECK(FlooredLayoutPoint(PointF(1.0f / 128, -1.0f / 128)) == LayoutPoint(0, LayoutUnit(-1.0f / 64)));
    CHECK(CeiledLayoutPoint(PointF(1.0f / 128, -1.0f / 128)) == LayoutPoint(LayoutUnit(1.0f / 64), 0));

    // A size snapped at a fractional location keeps its far edge on the pixel grid.
    STATIC_CHECK(SnappedSizeI(LayoutSize(10, 10), LayoutPoint(LayoutUnit(0.5f), LayoutUnit(0.25f)))
                 == SizeI(10, 10));
    STATIC_CHECK(SnappedSizeI(LayoutSize(LayoutUnit(10.25f), 10), LayoutPoint(LayoutUnit(0.25f), 0))
                 == SizeI(11, 10));

    CHECK(RoundPointToDevicePixels(LayoutPoint(LayoutUnit(1.25f), LayoutUnit(1.75f)), 2.0f)
          == PointF(1.5f, 2.0f));
    CHECK(FloorPointToDevicePixels(LayoutPoint(LayoutUnit(1.25f), LayoutUnit(1.75f)), 2.0f)
          == PointF(1.0f, 1.5f));
    CHECK(CeilPointToDevicePixels(LayoutPoint(LayoutUnit(1.25f), LayoutUnit(1.75f)), 2.0f)
          == PointF(1.5f, 2.0f));
    CHECK(SnapSizeToDevicePixel(LayoutSize(LayoutUnit(1.25f), 1), LayoutPoint(LayoutUnit(0.25f), 0), 2.0f)
          == SizeF(1.0f, 1.0f));

    constexpr LayoutPointLimits limits {LayoutPoint(0, 0), LayoutPoint(10, 10)};
    STATIC_CHECK(limits.Clamp(LayoutPoint(20, -5)) == LayoutPoint(10, 0));
    STATIC_CHECK(limits.Fits(LayoutPoint(5, 5)));
    STATIC_CHECK(limits.Distance(LayoutPoint(20, -5)) == LayoutSize(10, -5));
  }

  TEST_CASE("Blend: layout units and points", "[Graphics][AnimationUtilities]")
  {
    const BlendingContext context(0.25);
    CHECK(Blend(LayoutUnit(0), LayoutUnit(4), context) == LayoutUnit(1));
    CHECK(Blend(PointI(0, 4), PointI(4, 8), context) == PointI(1, 5));
    CHECK(Blend(PointF(0.0f, 4.0f), PointF(4.0f, 8.0f), context) == PointF(1.0f, 5.0f));
    CHECK(Blend(LayoutPoint(0, 4), LayoutPoint(4, 8), context) == LayoutPoint(1, 5));
  }

  TEST_CASE("LayoutPoint: conversions to and from PointD", "[Graphics][LayoutPoint]")
  {
    CHECK(LayoutPoint(PointD(1.5, 2.25)) == LayoutPoint(LayoutUnit(1.5), LayoutUnit(2.25)));
    PointD fromLayout = LayoutPoint(LayoutUnit(1.5), LayoutUnit(-2.25));
    CHECK(fromLayout == PointD(1.5, -2.25));
  }
}

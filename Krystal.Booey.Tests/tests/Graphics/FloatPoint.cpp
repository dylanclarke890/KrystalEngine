#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include <numbers>

namespace krys::boo::tests
{
  using Catch::Matchers::WithinAbs;

  TEST_CASE("FloatPoint: construction", "[Graphics][FloatPoint]")
  {
    constexpr FloatPoint defaultConstructed;
    STATIC_CHECK(defaultConstructed.IsZero());
    STATIC_CHECK(FloatPoint::Zero() == defaultConstructed);
    STATIC_CHECK(FloatPoint(IntPoint(2, 3)) == FloatPoint(2.0f, 3.0f));
    STATIC_CHECK(FloatPoint(FloatSize(2.0f, 3.0f)) == FloatPoint(2.0f, 3.0f));
    STATIC_CHECK(FloatPoint::NarrowPrecision(2.0, 3.0) == FloatPoint(2.0f, 3.0f));
    STATIC_CHECK_FALSE(FloatPoint(0.0f, 1.0f).IsZero());

    FloatPoint point;
    point.SetX(1.0f);
    point.SetY(2.0f);
    CHECK(point == FloatPoint(1.0f, 2.0f));
    point.Set(3.0f, 4.0f);
    CHECK(point == FloatPoint(3.0f, 4.0f));
  }

  TEST_CASE("FloatPoint: move, scale and rotate", "[Graphics][FloatPoint]")
  {
    FloatPoint point(1.0f, 2.0f);
    point.Move(1.0f, 1.0f);
    CHECK(point == FloatPoint(2.0f, 3.0f));
    point.Move(IntSize(1, 1));
    CHECK(point == FloatPoint(3.0f, 4.0f));
    point.Move(FloatSize(0.5f, 0.5f));
    CHECK(point == FloatPoint(3.5f, 4.5f));
    point.MoveBy(IntPoint(1, 1));
    CHECK(point == FloatPoint(4.5f, 5.5f));
    point.MoveBy(FloatPoint(-0.5f, -0.5f));
    CHECK(point == FloatPoint(4.0f, 5.0f));
    point.Scale(2.0f);
    CHECK(point == FloatPoint(8.0f, 10.0f));
    point.Scale(0.5f, 0.1f);
    CHECK(point == FloatPoint(4.0f, 1.0f));

    STATIC_CHECK(FloatPoint(1.0f, 2.0f).Scaled(2.0f) == FloatPoint(2.0f, 4.0f));
    STATIC_CHECK(FloatPoint(1.0f, 2.0f).Scaled(2.0f, 3.0f) == FloatPoint(2.0f, 6.0f));
    STATIC_CHECK(FloatPoint(1.0f, 2.0f).ScaledBy(2.0f) == FloatPoint(2.0f, 4.0f));

    FloatPoint rotated(1.0f, 0.0f);
    rotated.Rotate(std::numbers::pi / 2);
    CHECK_THAT(rotated.X(), WithinAbs(0.0f, 1e-6f));
    CHECK_THAT(rotated.Y(), WithinAbs(1.0f, 1e-6f));

    FloatPoint rotatedAbout(2.0f, 1.0f);
    rotatedAbout.Rotate(std::numbers::pi, FloatPoint(1.0f, 1.0f));
    CHECK_THAT(rotatedAbout.X(), WithinAbs(0.0f, 1e-6f));
    CHECK_THAT(rotatedAbout.Y(), WithinAbs(1.0f, 1e-6f));
  }

  TEST_CASE("FloatPoint: geometry", "[Graphics][FloatPoint]")
  {
    STATIC_CHECK(FloatPoint(3.0f, 4.0f).LengthSquared() == 25.0f);
    CHECK(FloatPoint(3.0f, 4.0f).Length() == 5.0f);
    STATIC_CHECK(FloatPoint(1.0f, 2.0f).Dot(FloatPoint(3.0f, 4.0f)) == 11.0f);
    STATIC_CHECK(FloatPoint(1.0f, 2.0f) * FloatPoint(3.0f, 4.0f) == 11.0f);
    CHECK_THAT(FloatPoint(1.0f, 1.0f).SlopeAngleRadians(), WithinAbs(std::numbers::pi_v<float> / 4, 1e-6f));

    FloatPoint normalised(3.0f, 4.0f);
    normalised.Normalise();
    CHECK_THAT(normalised.X(), WithinAbs(0.6f, 1e-6f));
    CHECK_THAT(normalised.Y(), WithinAbs(0.8f, 1e-6f));
    FloatPoint zero;
    zero.Normalise();
    CHECK(zero.IsZero());

    constexpr FloatPoint a(1.0f, 4.0f);
    constexpr FloatPoint b(3.0f, 2.0f);
    STATIC_CHECK(a.ExpandedTo(b) == FloatPoint(3.0f, 4.0f));
    STATIC_CHECK(a.ShrunkTo(b) == FloatPoint(1.0f, 2.0f));
    STATIC_CHECK(a.TransposedPoint() == FloatPoint(4.0f, 1.0f));
    STATIC_CHECK(FloatPoint(0.0f, 5.0f).ConstrainedBetween({1.0f, 1.0f}, {3.0f, 3.0f})
                 == FloatPoint(1.0f, 3.0f));

    CHECK(FloatPoint::NanPoint().IsNaN());
    STATIC_CHECK_FALSE(a.IsNaN());
  }

  TEST_CASE("FloatPoint: arithmetic and conversions", "[Graphics][FloatPoint]")
  {
    STATIC_CHECK(FloatPoint(1.0f, 2.0f) + FloatSize(1.0f, 1.0f) == FloatPoint(2.0f, 3.0f));
    STATIC_CHECK(FloatPoint(1.0f, 2.0f) + FloatPoint(1.0f, 1.0f) == FloatPoint(2.0f, 3.0f));
    STATIC_CHECK(FloatPoint(1.0f, 2.0f) - FloatPoint(1.0f, 1.0f) == FloatSize(0.0f, 1.0f));
    STATIC_CHECK(FloatPoint(1.0f, 2.0f) - FloatSize(1.0f, 1.0f) == FloatPoint(0.0f, 1.0f));
    STATIC_CHECK(-FloatPoint(1.0f, 2.0f) == FloatPoint(-1.0f, -2.0f));

    FloatPoint point(1.0f, 2.0f);
    point += FloatSize(1.0f, 1.0f);
    CHECK(point == FloatPoint(2.0f, 3.0f));
    point += FloatPoint(1.0f, 1.0f);
    CHECK(point == FloatPoint(3.0f, 4.0f));
    point -= FloatSize(3.0f, 3.0f);
    CHECK(point == FloatPoint(0.0f, 1.0f));

    STATIC_CHECK(ToFloatSize(FloatPoint(1.0f, 2.0f)) == FloatSize(1.0f, 2.0f));
    STATIC_CHECK(ToFloatPoint(FloatSize(1.0f, 2.0f)) == FloatPoint(1.0f, 2.0f));

    CHECK(FlooredIntSize(FloatPoint(1.5f, -1.5f)) == IntSize(1, -2));
    CHECK(RoundedIntPoint(FloatPoint(1.5f, -1.4f)) == IntPoint(2, -1));
    CHECK(FlooredIntPoint(FloatPoint(1.5f, -1.4f)) == IntPoint(1, -2));
    CHECK(CeiledIntPoint(FloatPoint(1.5f, -1.4f)) == IntPoint(2, -1));
    CHECK(CeiledIntPoint(FloatPoint(1e20f, 0.0f)) == IntPoint(Limits<int32>::max(), 0));
    CHECK(FloorPointToDevicePixels(FloatPoint(1.3f, 1.8f), 2.0f) == FloatPoint(1.0f, 1.5f));
    CHECK(CeilPointToDevicePixels(FloatPoint(1.3f, 1.8f), 2.0f) == FloatPoint(1.5f, 2.0f));

    CHECK(AreEssentiallyEqual(FloatPoint(1.0f, 2.0f), FloatPoint(1.0f, 2.0f)));
    CHECK_FALSE(AreEssentiallyEqual(FloatPoint(1.0f, 2.0f), FloatPoint(1.0f, 2.1f)));
  }

  TEST_CASE("FloatPoint: hash and markable traits", "[Graphics][FloatPoint]")
  {
    CHECK(DefaultHash<FloatPoint>::Hash(FloatPoint(1.0f, 2.0f))
          == DefaultHash<FloatPoint>::Hash(FloatPoint(1.0f, 2.0f)));
    CHECK(DefaultHash<FloatPoint>::Hash(FloatPoint(1.0f, 2.0f))
          != DefaultHash<FloatPoint>::Hash(FloatPoint(2.0f, 1.0f)));
    CHECK(DefaultHash<FloatPoint>::Equal(FloatPoint(1.0f, 2.0f), FloatPoint(1.0f, 2.0f)));
    CHECK(MarkableTraits<FloatPoint>::IsEmptyValue(MarkableTraits<FloatPoint>::EmptyValue()));
    CHECK_FALSE(MarkableTraits<FloatPoint>::IsEmptyValue(FloatPoint()));
  }
}

#include "Krystal.Core/Geometry/Point.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include <numbers>

namespace krys::tests
{
  namespace
  {
    void TestGetAndSet(PointI point)
    {
      point.SetX(1);
      CHECK(1 == point.X());
      point.SetY(2);
      CHECK(2 == point.Y());
    }
  }

  TEST_CASE("PointI: construction", "[Geometry][PointI]")
  {
    constexpr PointI defaultConstructed;
    STATIC_CHECK(defaultConstructed.X() == 0);
    STATIC_CHECK(defaultConstructed.Y() == 0);
    STATIC_CHECK(defaultConstructed.IsZero());
    TestGetAndSet(defaultConstructed);

    constexpr PointI value(10, 9);
    STATIC_CHECK(value.X() == 10);
    STATIC_CHECK(value.Y() == 9);
    TestGetAndSet(value);

    STATIC_CHECK(PointI::Zero().IsZero());

    constexpr PointI fromSize(SizeI(2'003, 1'997));
    STATIC_CHECK(fromSize.X() == 2'003);
    STATIC_CHECK(fromSize.Y() == 1'997);
    STATIC_CHECK_FALSE(fromSize.IsZero());

    PointI fromFloat(PointF(2003.2f, 1997.3f));
    CHECK(fromFloat.X() == 2'003);
    CHECK(fromFloat.Y() == 1'997);
    CHECK_FALSE(fromFloat.IsZero());

    PointI clamped(PointF(1e20f, -1e20f));
    CHECK(clamped.X() == Limits<int32>::max());
    CHECK(clamped.Y() == Limits<int32>::min());
  }

  TEST_CASE("PointI: move and scale", "[Geometry][PointI]")
  {
    PointI test(10, 20);
    test.Move(SizeI(30, 50));
    CHECK(test == PointI(40, 70));
    test.Move(-2, 8);
    CHECK(test == PointI(38, 78));
    test.MoveBy(PointI(100, 100));
    CHECK(test == PointI(138, 178));

    PointI scaled(10, 20);
    scaled.Scale(2.0f);
    CHECK(scaled == PointI(20, 40));
    scaled.Scale(3.0f, 1.5f);
    CHECK(scaled == PointI(60, 60));

    CHECK(PointI(10, 20).Scaled(1.25f) == PointI(13, 25));
    CHECK(PointI(10, 20).Scaled(0.5f, 2.0f) == PointI(5, 40));
  }

  TEST_CASE("PointI: expand, shrink, constrain and transpose", "[Geometry][PointI]")
  {
    constexpr PointI a(10, 20);
    constexpr PointI b(20, 40);
    STATIC_CHECK(a.ExpandedTo(b) == PointI(20, 40));
    STATIC_CHECK(b.ShrunkTo(a) == PointI(10, 20));
    STATIC_CHECK(a.TransposedPoint() == PointI(20, 10));
    STATIC_CHECK(PointI(0, 50).ConstrainedBetween(a, b) == PointI(10, 40));
    STATIC_CHECK(a.DistanceSquaredToPoint(b) == 500);

    PointI negative(-5, 5);
    negative.ClampNegativeToZero();
    CHECK(negative == PointI(0, 5));
  }

  TEST_CASE("PointI: arithmetic", "[Geometry][PointI]")
  {
    STATIC_CHECK(ToSizeI(PointI(10, 20)) == SizeI(10, 20));

    PointI a(10, 20);
    STATIC_CHECK(PointI(10, 20) + PointI(50, 60) == PointI(60, 80));
    STATIC_CHECK(PointI(10, 20) + SizeI(50, 60) == PointI(60, 80));
    a += SizeI(50, 60);
    CHECK(a == PointI(60, 80));

    PointI b(100, 80);
    STATIC_CHECK(PointI(100, 80) - PointI(50, 60) == SizeI(50, 20));
    STATIC_CHECK(PointI(100, 80) - SizeI(50, 60) == PointI(50, 20));
    b -= SizeI(50, 60);
    CHECK(b == PointI(50, 20));

    STATIC_CHECK(-PointI(100, 80) == PointI(-100, -80));

    STATIC_CHECK(PointI(100, 80) == PointI(100, 80));
    STATIC_CHECK(PointI(100, 80) != PointI(70, 50));
  }

  using Catch::Matchers::WithinAbs;

  TEST_CASE("PointF: construction", "[Geometry][PointF]")
  {
    constexpr PointF defaultConstructed;
    STATIC_CHECK(defaultConstructed.IsZero());
    STATIC_CHECK(PointF::Zero() == defaultConstructed);
    STATIC_CHECK(PointF(PointI(2, 3)) == PointF(2.0f, 3.0f));
    STATIC_CHECK(PointF(SizeF(2.0f, 3.0f)) == PointF(2.0f, 3.0f));
    STATIC_CHECK(PointF::NarrowPrecision(2.0, 3.0) == PointF(2.0f, 3.0f));
    STATIC_CHECK_FALSE(PointF(0.0f, 1.0f).IsZero());

    PointF point;
    point.SetX(1.0f);
    point.SetY(2.0f);
    CHECK(point == PointF(1.0f, 2.0f));
    point.Set(3.0f, 4.0f);
    CHECK(point == PointF(3.0f, 4.0f));
  }

  TEST_CASE("PointF: move, scale and rotate", "[Geometry][PointF]")
  {
    PointF point(1.0f, 2.0f);
    point.Move(1.0f, 1.0f);
    CHECK(point == PointF(2.0f, 3.0f));
    point.Move(SizeI(1, 1));
    CHECK(point == PointF(3.0f, 4.0f));
    point.Move(SizeF(0.5f, 0.5f));
    CHECK(point == PointF(3.5f, 4.5f));
    point.MoveBy(PointI(1, 1));
    CHECK(point == PointF(4.5f, 5.5f));
    point.MoveBy(PointF(-0.5f, -0.5f));
    CHECK(point == PointF(4.0f, 5.0f));
    point.Scale(2.0f);
    CHECK(point == PointF(8.0f, 10.0f));
    point.Scale(0.5f, 0.1f);
    CHECK(point == PointF(4.0f, 1.0f));

    STATIC_CHECK(PointF(1.0f, 2.0f).Scaled(2.0f) == PointF(2.0f, 4.0f));
    STATIC_CHECK(PointF(1.0f, 2.0f).Scaled(2.0f, 3.0f) == PointF(2.0f, 6.0f));
    STATIC_CHECK(PointF(1.0f, 2.0f).ScaledBy(2.0f) == PointF(2.0f, 4.0f));

    PointF rotated(1.0f, 0.0f);
    rotated.Rotate(std::numbers::pi / 2);
    CHECK_THAT(rotated.X(), WithinAbs(0.0f, 1e-6f));
    CHECK_THAT(rotated.Y(), WithinAbs(1.0f, 1e-6f));

    PointF rotatedAbout(2.0f, 1.0f);
    rotatedAbout.Rotate(std::numbers::pi, PointF(1.0f, 1.0f));
    CHECK_THAT(rotatedAbout.X(), WithinAbs(0.0f, 1e-6f));
    CHECK_THAT(rotatedAbout.Y(), WithinAbs(1.0f, 1e-6f));
  }

  TEST_CASE("PointF: geometry", "[Geometry][PointF]")
  {
    STATIC_CHECK(PointF(3.0f, 4.0f).LengthSquared() == 25.0f);
    CHECK(PointF(3.0f, 4.0f).Length() == 5.0f);
    STATIC_CHECK(PointF(1.0f, 2.0f).Dot(PointF(3.0f, 4.0f)) == 11.0f);
    STATIC_CHECK(PointF(1.0f, 2.0f) * PointF(3.0f, 4.0f) == 11.0f);
    CHECK_THAT(PointF(1.0f, 1.0f).SlopeAngleRadians(), WithinAbs(std::numbers::pi_v<float> / 4, 1e-6f));

    PointF normalised(3.0f, 4.0f);
    normalised.Normalise();
    CHECK_THAT(normalised.X(), WithinAbs(0.6f, 1e-6f));
    CHECK_THAT(normalised.Y(), WithinAbs(0.8f, 1e-6f));
    PointF zero;
    zero.Normalise();
    CHECK(zero.IsZero());

    constexpr PointF a(1.0f, 4.0f);
    constexpr PointF b(3.0f, 2.0f);
    STATIC_CHECK(a.ExpandedTo(b) == PointF(3.0f, 4.0f));
    STATIC_CHECK(a.ShrunkTo(b) == PointF(1.0f, 2.0f));
    STATIC_CHECK(a.TransposedPoint() == PointF(4.0f, 1.0f));
    STATIC_CHECK(PointF(0.0f, 5.0f).ConstrainedBetween({1.0f, 1.0f}, {3.0f, 3.0f}) == PointF(1.0f, 3.0f));

    CHECK(PointF::NanPoint().IsNaN());
    STATIC_CHECK_FALSE(a.IsNaN());
  }

  TEST_CASE("PointF: arithmetic and conversions", "[Geometry][PointF]")
  {
    STATIC_CHECK(PointF(1.0f, 2.0f) + SizeF(1.0f, 1.0f) == PointF(2.0f, 3.0f));
    STATIC_CHECK(PointF(1.0f, 2.0f) + PointF(1.0f, 1.0f) == PointF(2.0f, 3.0f));
    STATIC_CHECK(PointF(1.0f, 2.0f) - PointF(1.0f, 1.0f) == SizeF(0.0f, 1.0f));
    STATIC_CHECK(PointF(1.0f, 2.0f) - SizeF(1.0f, 1.0f) == PointF(0.0f, 1.0f));
    STATIC_CHECK(-PointF(1.0f, 2.0f) == PointF(-1.0f, -2.0f));

    PointF point(1.0f, 2.0f);
    point += SizeF(1.0f, 1.0f);
    CHECK(point == PointF(2.0f, 3.0f));
    point += PointF(1.0f, 1.0f);
    CHECK(point == PointF(3.0f, 4.0f));
    point -= SizeF(3.0f, 3.0f);
    CHECK(point == PointF(0.0f, 1.0f));

    STATIC_CHECK(ToSizeF(PointF(1.0f, 2.0f)) == SizeF(1.0f, 2.0f));
    STATIC_CHECK(ToPointF(SizeF(1.0f, 2.0f)) == PointF(1.0f, 2.0f));

    CHECK(FlooredSizeI(PointF(1.5f, -1.5f)) == SizeI(1, -2));
    CHECK(RoundedPointI(PointF(1.5f, -1.4f)) == PointI(2, -1));
    CHECK(FlooredPointI(PointF(1.5f, -1.4f)) == PointI(1, -2));
    CHECK(CeiledPointI(PointF(1.5f, -1.4f)) == PointI(2, -1));
    CHECK(CeiledPointI(PointF(1e20f, 0.0f)) == PointI(Limits<int32>::max(), 0));
    CHECK(FloorPointToDevicePixels(PointF(1.3f, 1.8f), 2.0f) == PointF(1.0f, 1.5f));
    CHECK(CeilPointToDevicePixels(PointF(1.3f, 1.8f), 2.0f) == PointF(1.5f, 2.0f));

    CHECK(AreEssentiallyEqual(PointF(1.0f, 2.0f), PointF(1.0f, 2.0f)));
    CHECK_FALSE(AreEssentiallyEqual(PointF(1.0f, 2.0f), PointF(1.0f, 2.1f)));
  }

  TEST_CASE("PointF: hash and markable traits", "[Geometry][PointF]")
  {
    CHECK(DefaultHash<PointF>::Hash(PointF(1.0f, 2.0f)) == DefaultHash<PointF>::Hash(PointF(1.0f, 2.0f)));
    CHECK(DefaultHash<PointF>::Hash(PointF(1.0f, 2.0f)) != DefaultHash<PointF>::Hash(PointF(2.0f, 1.0f)));
    CHECK(DefaultHash<PointF>::Equal(PointF(1.0f, 2.0f), PointF(1.0f, 2.0f)));
    CHECK(MarkableTraits<PointF>::IsEmptyValue(MarkableTraits<PointF>::EmptyValue()));
    CHECK_FALSE(MarkableTraits<PointF>::IsEmptyValue(PointF()));
  }

  TEST_CASE("SizeD", "[Geometry][SizeD]")
  {
    STATIC_CHECK(SizeD() == SizeD(0, 0));
    STATIC_CHECK(SizeD(2, 3).Width() == 2);
    STATIC_CHECK(SizeD(2, 3).Height() == 3);
    STATIC_CHECK(SizeD(2, 3).ScaledBy(2) == SizeD(4, 6));
    STATIC_CHECK(SizeD(2, 3).ScaledBy(2, 0.5) == SizeD(4, 1.5));

    CHECK(SizeD(SizeF(1.5f, -2.5f)) == SizeD(1.5, -2.5));
    SizeD converted = SizeF(1.5f, 2.5f);
    CHECK(converted == SizeD(1.5, 2.5));
  }

  TEST_CASE("PointD: construction", "[Geometry][PointD]")
  {
    STATIC_CHECK(PointD().IsZero());
    STATIC_CHECK(PointD::Zero() == PointD(0, 0));
    STATIC_CHECK_FALSE(PointD(0, 1).IsZero());
    STATIC_CHECK(PointD(PointI(3, -4)) == PointD(3, -4));
    STATIC_CHECK(PointD(SizeD(1.25, 2.5)) == PointD(1.25, 2.5));

    // Conversions to and from float points.
    CHECK(PointF(PointD(1.5, 2.25)) == PointF(1.5f, 2.25f));
    PointD fromFloat = PointF(1.5f, 2.25f);
    CHECK(fromFloat == PointD(1.5, 2.25));
  }

  TEST_CASE("PointD: move and scale", "[Geometry][PointD]")
  {
    PointD p(1, 2);
    p.Move(0.5, 0.25);
    CHECK(p == PointD(1.5, 2.25));
    p.Move(SizeI(1, 1));
    CHECK(p == PointD(2.5, 3.25));
    p.Move(SizeD(-0.5, -0.25));
    CHECK(p == PointD(2, 3));
    p.MoveBy(PointD(1, 1));
    CHECK(p == PointD(3, 4));
    p.Scale(2);
    CHECK(p == PointD(6, 8));
    CHECK(p.Scaled(0.5) == PointD(3, 4));
  }

  TEST_CASE("PointD: arithmetic and rounding", "[Geometry][PointD]")
  {
    STATIC_CHECK(PointD(1, 2) + SizeD(3, 4) == PointD(4, 6));
    STATIC_CHECK(PointD(1, 2) + PointD(3, 4) == PointD(4, 6));
    STATIC_CHECK(PointD(1, 2) - PointD(3, 5) == SizeD(-2, -3));
    STATIC_CHECK(PointD(1, 2) - SizeD(3, 5) == PointD(-2, -3));
    STATIC_CHECK(ToSizeD(PointD(1, 2)) == SizeD(1, 2));

    CHECK(FlooredPointI(PointD(1.7, -1.2)) == PointI(1, -2));
    CHECK(RoundedPointI(PointD(1.5, -1.2)) == PointI(2, -1));
    CHECK(RoundedPointI(PointD(1e20, -1e20)) == PointI(Limits<int>::max(), Limits<int>::min()));
  }
}

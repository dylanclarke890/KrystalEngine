#include "Krystal.Booey/Graphics/FloatPoint3D.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

namespace krys::boo::tests
{
  using Catch::Matchers::WithinAbs;

  TEST_CASE("FloatPoint3D: construction and accessors", "[Graphics][FloatPoint3D]")
  {
    STATIC_CHECK(FloatPoint3D().IsZero());
    STATIC_CHECK(FloatPoint3D(FloatPoint(1, 2)) == FloatPoint3D(1, 2, 0));
    STATIC_CHECK(FloatPoint3D(1, 2, 3).XY() == FloatPoint(1, 2));

    FloatPoint3D p;
    p.Set(1, 2, 3);
    CHECK(p == FloatPoint3D(1, 2, 3));
    p.SetXY(FloatPoint(4, 5));
    p.SetZ(6);
    CHECK(p == FloatPoint3D(4, 5, 6));
    p.Move(1, 1);
    CHECK(p == FloatPoint3D(5, 6, 6));
    p.Move(0, 0, 1);
    p.Scale(2, 3, 0.5f);
    CHECK(p == FloatPoint3D(10, 18, 3.5f));
  }

  TEST_CASE("FloatPoint3D: vector maths", "[Graphics][FloatPoint3D]")
  {
    constexpr FloatPoint3D x(1, 0, 0);
    constexpr FloatPoint3D y(0, 1, 0);

    STATIC_CHECK(x.Dot(y) == 0);
    STATIC_CHECK(x * FloatPoint3D(2, 3, 4) == 2);
    STATIC_CHECK(x.Cross(y) == FloatPoint3D(0, 0, 1));
    STATIC_CHECK(y.Cross(x) == FloatPoint3D(0, 0, -1));
    STATIC_CHECK(FloatPoint3D(1, 2, 2).LengthSquared() == 9);

    CHECK(FloatPoint3D(1, 2, 2).Length() == 3);
    CHECK(FloatPoint3D(1, 2, 2).DistanceTo(FloatPoint3D(1, 2, 0)) == 2);

    // Crossing into one of the arguments is safe.
    FloatPoint3D a = x;
    a.Cross(a, y);
    CHECK(a == FloatPoint3D(0, 0, 1));

    FloatPoint3D v(3, 0, 4);
    v.Normalise();
    CHECK_THAT(v.X(), WithinAbs(0.6, 1e-6));
    CHECK_THAT(v.Z(), WithinAbs(0.8, 1e-6));

    // A zero vector stays zero.
    FloatPoint3D zero;
    zero.Normalise();
    CHECK(zero.IsZero());
  }

  TEST_CASE("FloatPoint3D: operators", "[Graphics][FloatPoint3D]")
  {
    STATIC_CHECK(FloatPoint3D(1, 2, 3) + FloatPoint3D(1, 1, 1) == FloatPoint3D(2, 3, 4));
    STATIC_CHECK(FloatPoint3D(1, 2, 3) + FloatPoint(1, 1) == FloatPoint3D(2, 3, 3));
    STATIC_CHECK(FloatPoint3D(1, 2, 3) - FloatPoint3D(1, 1, 1) == FloatPoint3D(0, 1, 2));
    STATIC_CHECK(FloatPoint3D(1, 2, 3) - FloatPoint(1, 1) == FloatPoint3D(0, 1, 3));
    STATIC_CHECK(2 * FloatPoint3D(1, 2, 3) == FloatPoint3D(2, 4, 6));
    STATIC_CHECK(FloatPoint3D(1, 2, 3) * 2 == FloatPoint3D(2, 4, 6));

    FloatPoint3D p(1, 2, 3);
    p += FloatPoint3D(1, 1, 1);
    p += FloatPoint(1, 1);
    CHECK(p == FloatPoint3D(3, 4, 4));
    p -= FloatPoint3D(1, 1, 1);
    p -= FloatPoint(1, 1);
    CHECK(p == FloatPoint3D(1, 2, 3));
  }
}

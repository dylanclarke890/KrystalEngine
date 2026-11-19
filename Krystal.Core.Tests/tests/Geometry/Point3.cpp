#include "Krystal.Core/Geometry/Point3.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

namespace krys::tests
{
  using Catch::Matchers::WithinAbs;

  TEST_CASE("Point3F: construction and accessors", "[Geometry][Point3F]")
  {
    STATIC_CHECK(Point3F().IsZero());
    STATIC_CHECK(Point3F(PointF(1, 2)) == Point3F(1, 2, 0));
    STATIC_CHECK(Point3F(1, 2, 3).XY() == PointF(1, 2));

    Point3F p;
    p.Set(1, 2, 3);
    CHECK(p == Point3F(1, 2, 3));
    p.SetXY(PointF(4, 5));
    p.SetZ(6);
    CHECK(p == Point3F(4, 5, 6));
    p.Move(1, 1);
    CHECK(p == Point3F(5, 6, 6));
    p.Move(0, 0, 1);
    p.Scale(2, 3, 0.5f);
    CHECK(p == Point3F(10, 18, 3.5f));
  }

  TEST_CASE("Point3F: vector maths", "[Geometry][Point3F]")
  {
    constexpr Point3F x(1, 0, 0);
    constexpr Point3F y(0, 1, 0);

    STATIC_CHECK(x.Dot(y) == 0);
    STATIC_CHECK(x * Point3F(2, 3, 4) == 2);
    STATIC_CHECK(x.Cross(y) == Point3F(0, 0, 1));
    STATIC_CHECK(y.Cross(x) == Point3F(0, 0, -1));
    STATIC_CHECK(Point3F(1, 2, 2).LengthSquared() == 9);

    CHECK(Point3F(1, 2, 2).Length() == 3);
    CHECK(Point3F(1, 2, 2).DistanceTo(Point3F(1, 2, 0)) == 2);

    // Crossing into one of the arguments is safe.
    Point3F a = x;
    a.Cross(a, y);
    CHECK(a == Point3F(0, 0, 1));

    Point3F v(3, 0, 4);
    v.Normalise();
    CHECK_THAT(v.X(), WithinAbs(0.6, 1e-6));
    CHECK_THAT(v.Z(), WithinAbs(0.8, 1e-6));

    // A zero vector stays zero.
    Point3F zero;
    zero.Normalise();
    CHECK(zero.IsZero());
  }

  TEST_CASE("Point3F: operators", "[Geometry][Point3F]")
  {
    STATIC_CHECK(Point3F(1, 2, 3) + Point3F(1, 1, 1) == Point3F(2, 3, 4));
    STATIC_CHECK(Point3F(1, 2, 3) + PointF(1, 1) == Point3F(2, 3, 3));
    STATIC_CHECK(Point3F(1, 2, 3) - Point3F(1, 1, 1) == Point3F(0, 1, 2));
    STATIC_CHECK(Point3F(1, 2, 3) - PointF(1, 1) == Point3F(0, 1, 3));
    STATIC_CHECK(2 * Point3F(1, 2, 3) == Point3F(2, 4, 6));
    STATIC_CHECK(Point3F(1, 2, 3) * 2 == Point3F(2, 4, 6));

    Point3F p(1, 2, 3);
    p += Point3F(1, 1, 1);
    p += PointF(1, 1);
    CHECK(p == Point3F(3, 4, 4));
    p -= Point3F(1, 1, 1);
    p -= PointF(1, 1);
    CHECK(p == Point3F(1, 2, 3));
  }
}

#include "Krystal.Maths/Clamp.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Maths;

  TEST_CASE("Clamp", "[Maths]")
  {
    REQUIRE(Clamp(5, 0, 10) == 5);

    REQUIRE(Clamp(Vec4(1, 2, 3, 4), Vec4(0), Vec4(10)) == Vec4(1, 2, 3, 4));
    REQUIRE(Clamp(Vec4(1, 2, 3, 4), 2.f, 5.f) == Vec4(2, 2, 3, 4));

    REQUIRE(Clamp(Mat2x2(1, 2, 3, 4), Mat2x2(0), Mat2x2(10)) == Mat2x2(1, 2, 3, 4));
    REQUIRE(Clamp(Mat2x2(1, 2, 3, 4), 2.f, 5.f) == Mat2x2(2, 2, 3, 4));
  }

  TEST_CASE("Min", "[Maths]")
  {
    REQUIRE(Min(5, 6) == 5);
    REQUIRE(Min(5.f, 6.f) == 5.f);

    REQUIRE(Min(Vec4(1, 2, 3, 4), Vec4(0)) == Vec4(0));
    REQUIRE(Min(Vec4(1, 2, 3, 4), Vec4(5)) == Vec4(1, 2, 3, 4));

    REQUIRE(Min(Mat2x2(1, 2, 3, 4), 2.f) == Mat2x2(1, 2, 2, 2));
    REQUIRE(Min(Mat2x2(1, 2, 3, 4), Mat2x2(0)) == Mat2x2(0));
  }

  TEST_CASE("Max", "[Maths]")
  {
    REQUIRE(Max(5, 6) == 6);
    REQUIRE(Max(5.f, 6.f) == 6.f);

    REQUIRE(Max(Vec4(1, 2, 3, 4), Vec4(0)) == Vec4(1, 2, 3, 4));
    REQUIRE(Max(Vec4(1, 2, 3, 4), Vec4(5)) == Vec4(5));

    REQUIRE(Max(Mat2x2(1, 2, 3, 4), 2.f) == Mat2x2(2, 2, 3, 4));
    REQUIRE(Max(Mat2x2(1, 2, 3, 4), Mat2x2(0)) == Mat2x2(1, 2, 3, 4));
  }

  TEST_CASE("FMin", "[Maths]")
  {
    REQUIRE(FMin(5.f, 6.f) == 5.f);
    REQUIRE(FMin(5.f, 6.f) == 5.f);

    REQUIRE(FMin(Vec4(1, 2, 3, 4), Vec4(0)) == Vec4(0));
    REQUIRE(FMin(Vec4(1, 2, 3, 4), Vec4(5)) == Vec4(1, 2, 3, 4));

    REQUIRE(FMin(Mat2x2(1, 2, 3, 4), 2.f) == Mat2x2(1, 2, 2, 2));
    REQUIRE(FMin(Mat2x2(1, 2, 3, 4), Mat2x2(0)) == Mat2x2(0));
  }

  TEST_CASE("FMax", "[Maths]")
  {
    REQUIRE(FMax(5.f, 6.f) == 6.f);
    REQUIRE(FMax(5.f, 6.f) == 6.f);
    REQUIRE(FMax(Vec4(1, 2, 3, 4), Vec4(0)) == Vec4(1, 2, 3, 4));
    REQUIRE(FMax(Vec4(1, 2, 3, 4), Vec4(5)) == Vec4(5));
    REQUIRE(FMax(Mat2x2(1, 2, 3, 4), 2.f) == Mat2x2(2, 2, 3, 4));
    REQUIRE(FMax(Mat2x2(1, 2, 3, 4), Mat2x2(0)) == Mat2x2(1, 2, 3, 4));
  }
}
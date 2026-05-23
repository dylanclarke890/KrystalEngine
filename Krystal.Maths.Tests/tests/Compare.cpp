#include "Krystal.Maths/Compare.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Maths;

  TEST_CASE("LessThan", "[Compare]")
  {
    REQUIRE(LessThan(Vec4i(1), Vec4i(2)) == Vec4b(true));
    REQUIRE(LessThan(Vec4i(2), Vec4i(1)) == Vec4b(false));
    REQUIRE(LessThan(Mat4x4i(1), Mat4x4i(1)) == Mat4x4b(false));
    REQUIRE(LessThan(Mat4x4i(1), Mat4x4i(2)) == Mat4x4b(true));
  }

  TEST_CASE("LessThanOrEqual", "[Compare]")
  {
    REQUIRE(LessThanOrEqual(Vec4i(1), Vec4i(2)) == Vec4b(true));
    REQUIRE(LessThanOrEqual(Vec4i(2), Vec4i(1)) == Vec4b(false));
    REQUIRE(LessThanOrEqual(Mat4x4i(1), Mat4x4i(1)) == Mat4x4b(true));
    REQUIRE(LessThanOrEqual(Mat4x4i(1), Mat4x4i(2)) == Mat4x4b(true));
  }

  TEST_CASE("Equal", "[Compare]")
  {
    REQUIRE(Equal(Vec4i(1), Vec4i(1)) == Vec4b(true));
    REQUIRE(Equal(Vec4i(1), Vec4i(2)) == Vec4b(false));
    REQUIRE(Equal(Mat4x4i(1), Mat4x4i(1)) == Mat4x4b(true));
    REQUIRE(Equal(Mat4x4i(1), Mat4x4i(2)) == Mat4x4b(false));
  }

  TEST_CASE("NotEqual", "[Compare]")
  {
    REQUIRE(NotEqual(Vec4i(1), Vec4i(1)) == Vec4b(false));
    REQUIRE(NotEqual(Vec4i(1), Vec4i(2)) == Vec4b(true));
    REQUIRE(NotEqual(Mat4x4i(1), Mat4x4i(1)) == Mat4x4b(false));
    REQUIRE(NotEqual(Mat4x4i(1), Mat4x4i(2)) == Mat4x4b(true));
  }

  TEST_CASE("GreaterThan", "[Compare]")
  {
    REQUIRE(GreaterThan(Vec4i(2), Vec4i(1)) == Vec4b(true));
    REQUIRE(GreaterThan(Vec4i(1), Vec4i(2)) == Vec4b(false));
    REQUIRE(GreaterThan(Mat4x4i(2), Mat4x4i(1)) == Mat4x4b(true));
    REQUIRE(GreaterThan(Mat4x4i(1), Mat4x4i(1)) == Mat4x4b(false));
  }

  TEST_CASE("GreaterThanOrEqual", "[Compare]")
  {
    REQUIRE(GreaterThanOrEqual(Vec4i(2), Vec4i(1)) == Vec4b(true));
    REQUIRE(GreaterThanOrEqual(Vec4i(1), Vec4i(2)) == Vec4b(false));
    REQUIRE(GreaterThanOrEqual(Mat4x4i(2), Mat4x4i(1)) == Mat4x4b(true));
    REQUIRE(GreaterThanOrEqual(Mat4x4i(1), Mat4x4i(1)) == Mat4x4b(true));
  }
}
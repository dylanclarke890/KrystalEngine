#include "Krystal.Maths/Interpolate.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Maths;

  TEST_CASE("Lerp", "[Interpolation]")
  {
    REQUIRE(Lerp(10.0f, 20.0f, 0.0f) == 10.0f);
    REQUIRE(Lerp(10, 20, 0.5f) == 15.0f);
    REQUIRE(Lerp(10.0f, 20.0f, 1.0f) == 20.0f);
    REQUIRE(Lerp(10.0f, 20.0f, 0.5f) == 15.0f);

    REQUIRE(Lerp(Vec4(10.0f), Vec4(20.0f), 0.0f) == Vec4(10.0f));
    REQUIRE(Lerp(Vec4(10.0f), Vec4(20.0f), 0.5f) == Vec4(15.0f));
    REQUIRE(Lerp(Vec4(10.0f), Vec4(20.0f), 1.0f) == Vec4(20.0f));
    REQUIRE(Lerp(Vec4(10.0f), Vec4(20.0f), 0.5f) == Vec4(15.0f));

    REQUIRE(Lerp(Mat4(10.0f), Mat4(20.0f), 0.0f) == Mat4(10.0f));
    REQUIRE(Lerp(Mat4(10.0f), Mat4(20.0f), 0.5f) == Mat4(15.0f));
    REQUIRE(Lerp(Mat4(10.0f), Mat4(20.0f), 1.0f) == Mat4(20.0f));
    REQUIRE(Lerp(Mat4(10.0f), Mat4(20.0f), 0.5f) == Mat4(15.0f));
  }

  TEST_CASE("Smoothstep", "[Interpolation]")
  {
    constexpr float edge0 = 0.0f;
    constexpr float edge1 = 1.0f;

    REQUIRE(Smoothstep(edge0, edge1, -0.5f) == 0.0f);
    REQUIRE(Smoothstep(edge0, edge1, edge0) == 0.0f);
    REQUIRE(Smoothstep(edge0, edge1, 0.5f) == 0.5f);
    REQUIRE(Smoothstep(edge0, edge1, edge1) == 1.0f);
    REQUIRE(Smoothstep(edge0, edge1, 1.5f) == 1.0f);
  }

  TEST_CASE("Step", "[Interpolation]")
  {
    REQUIRE(Step(-2, 5, 0.5f) == 5);
    REQUIRE(Step(2, 9, 0.45f) == 2);
    REQUIRE(Step(10, 2) == 0);
  }
}
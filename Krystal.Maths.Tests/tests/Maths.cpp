#include "Krystal.Maths/Maths.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Maths;

  TEST_CASE("Abs", "[Maths]")
  {
    REQUIRE(Abs(-1.f) == 1.f);
    REQUIRE(Abs(1.f) == 1.f);

    REQUIRE(Abs(Vec4(-1.f)) == Abs(Vec4(-1.f)));
    REQUIRE(Abs(Vec4(1.f)) == Vec4(1.f));

    REQUIRE(Abs(Mat4(-1.f)) == Mat4(1.f));
    REQUIRE(Abs(Mat4(1.f)) == Mat4(1.f));
  }
}
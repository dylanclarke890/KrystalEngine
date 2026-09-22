#include "Krystal.Core/Maths/Maths.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::tests
{
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
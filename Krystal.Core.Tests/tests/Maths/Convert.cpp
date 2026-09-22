#include "Krystal.Core/Maths/Convert.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

namespace krys::tests
{
  TEST_CASE("DegreesToRadians", "[Convert]")
  {
    REQUIRE(krys::Radians(0.0) == 0.0);
    REQUIRE_THAT(krys::Radians(90.0), Catch::Matchers::WithinRel(1.5707963267948966));
    REQUIRE_THAT(krys::Radians(180.0), Catch::Matchers::WithinRel(3.141592653589793));
    REQUIRE_THAT(krys::Radians(360.0), Catch::Matchers::WithinRel(6.283185307179586));
    REQUIRE_THAT((double)360.0_radians, Catch::Matchers::WithinRel(6.283185307179586));
  }

  TEST_CASE("RadiansToDegrees", "[Convert]")
  {
    REQUIRE(krys::Degrees(0.0) == 0.0);
    REQUIRE_THAT(krys::Degrees(1.5707963267948966), Catch::Matchers::WithinRel(90.0));
    REQUIRE_THAT(krys::Degrees(3.141592653589793), Catch::Matchers::WithinRel(180.0));
    REQUIRE_THAT(krys::Degrees(6.283185307179586), Catch::Matchers::WithinRel(360.0));
    REQUIRE_THAT((double)6.283185307179586_degrees, Catch::Matchers::WithinRel(360.0));
  }
}
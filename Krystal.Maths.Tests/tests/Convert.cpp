#include "Krystal.Maths/Convert.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Maths;
  using namespace Catch::Matchers;

  TEST_CASE("DegreesToRadians", "[Convert]")
  {
    REQUIRE(Radians(0.0) == 0.0);
    REQUIRE_THAT(Radians(90.0), WithinRel(1.5707963267948966));
    REQUIRE_THAT(Radians(180.0), WithinRel(3.141592653589793));
    REQUIRE_THAT(Radians(360.0), WithinRel(6.283185307179586));
    REQUIRE_THAT((double)360.0_radians, WithinRel(6.283185307179586));
  }

  TEST_CASE("RadiansToDegrees", "[Convert]")
  {
    REQUIRE(Degrees(0.0) == 0.0);
    REQUIRE_THAT(Degrees(1.5707963267948966), WithinRel(90.0));
    REQUIRE_THAT(Degrees(3.141592653589793), WithinRel(180.0));
    REQUIRE_THAT(Degrees(6.283185307179586), WithinRel(360.0));
    REQUIRE_THAT((double)6.283185307179586_degrees, WithinRel(360.0));
  }
}
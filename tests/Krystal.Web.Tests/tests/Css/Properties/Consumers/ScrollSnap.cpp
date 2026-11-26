#include "Krystal.Web/Css/Properties/Consumers/ScrollSnap.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ConsumerTestUtils.hpp"

namespace krys::web::css::parse_property::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("parse_property(ConsumeScrollSnapType)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeScrollSnapType) == "none");
    CHECK(ConsumeToText(u8"x", ConsumeScrollSnapType) == "x");
    CHECK(ConsumeToText(u8"both mandatory", ConsumeScrollSnapType) == "both mandatory");

    // The initial strictness isn't kept.
    CHECK(ConsumeToText(u8"block proximity", ConsumeScrollSnapType) == "block");

    CHECK(ConsumeToText(u8"mandatory", ConsumeScrollSnapType) == "<null>");
    CHECK(ConsumeToText(u8"none mandatory", ConsumeScrollSnapType) == "<partial>");
  }
}

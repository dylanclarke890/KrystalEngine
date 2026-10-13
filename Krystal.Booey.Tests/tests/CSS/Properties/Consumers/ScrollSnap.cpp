#include "Krystal.Booey/CSS/Properties/Consumers/ScrollSnap.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeScrollSnapType)", "[CSS][Properties][Consumers]")
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

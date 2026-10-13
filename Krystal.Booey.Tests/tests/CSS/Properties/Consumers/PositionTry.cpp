#include "Krystal.Booey/CSS/Properties/Consumers/PositionTry.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumePositionTryFallbacks)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumePositionTryFallbacks) == "none");
    CHECK(ConsumeToText(u8"--foo", ConsumePositionTryFallbacks) == "--foo");
    CHECK(ConsumeToText(u8"flip-block", ConsumePositionTryFallbacks) == "flip-block");
    CHECK(ConsumeToText(u8"--foo flip-x flip-y", ConsumePositionTryFallbacks) == "--foo flip-x flip-y");

    // The rule name comes first, whichever order it's written in.
    CHECK(ConsumeToText(u8"flip-inline --foo", ConsumePositionTryFallbacks) == "--foo flip-inline");

    // <'position-area'>
    CHECK(ConsumeToText(u8"top left", ConsumePositionTryFallbacks) == "left top");
    CHECK(ConsumeToText(u8"--a, flip-start, bottom", ConsumePositionTryFallbacks)
          == "--a, flip-start, bottom");

    CHECK(ConsumeToText(u8"flip-x flip-x", ConsumePositionTryFallbacks) == "<null>");
    CHECK(ConsumeToText(u8"foo", ConsumePositionTryFallbacks) == "<null>");
    CHECK(ConsumeToText(u8"--a, none", ConsumePositionTryFallbacks) == "<null>");
    CHECK(ConsumeToText(u8"--a,", ConsumePositionTryFallbacks) == "<null>");
  }
}

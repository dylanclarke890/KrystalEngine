#include "Krystal.Booey/CSS/Properties/Consumers/ViewTransition.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeViewTransitionTypes)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeViewTransitionTypes) == "none");
    CHECK(ConsumeToText(u8"slide", ConsumeViewTransitionTypes) == "slide");
    CHECK(ConsumeToText(u8"slide fade", ConsumeViewTransitionTypes) == "slide fade");

    CHECK(ConsumeToText(u8"slide none", ConsumeViewTransitionTypes) == "<null>");
    CHECK(ConsumeToText(u8"-ua-slide", ConsumeViewTransitionTypes) == "<null>");
    CHECK(ConsumeToText(u8"slide initial", ConsumeViewTransitionTypes) == "<null>");
    CHECK(ConsumeToText(u8"slide, fade", ConsumeViewTransitionTypes) == "<null>");
  }
}

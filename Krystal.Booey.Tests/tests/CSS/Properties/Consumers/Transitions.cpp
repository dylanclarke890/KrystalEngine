#include "Krystal.Booey/CSS/Properties/Consumers/Transitions.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeSingleTransitionProperty)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"all", ConsumeSingleTransitionProperty) == "all");
    CHECK(ConsumeToText(u8"opacity", ConsumeSingleTransitionProperty) == "opacity");
    CHECK(ConsumeToText(u8"OPACITY", ConsumeSingleTransitionProperty) == "opacity");
    CHECK(ConsumeToText(u8"not-a-property", ConsumeSingleTransitionProperty) == "not-a-property");

    CHECK(ConsumeToText(u8"none", ConsumeSingleTransitionProperty) == "<null>");
    CHECK(ConsumeToText(u8"inherit", ConsumeSingleTransitionProperty) == "<null>");
    CHECK(ConsumeToText(u8"5", ConsumeSingleTransitionProperty) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeSingleTransitionPropertyOrNone)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeSingleTransitionPropertyOrNone) == "none");
    CHECK(ConsumeToText(u8"width", ConsumeSingleTransitionPropertyOrNone) == "width");
    CHECK(ConsumeToText(u8"\"width\"", ConsumeSingleTransitionPropertyOrNone) == "<null>");
  }
}

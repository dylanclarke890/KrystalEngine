#include "Krystal.Booey/CSS/Properties/Consumers/Lists.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeCounterReset)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeCounterReset) == "none");
    CHECK(ConsumeToText(u8"chapter", ConsumeCounterReset) == "chapter 0");
    CHECK(ConsumeToText(u8"chapter 3", ConsumeCounterReset) == "chapter 3");
    CHECK(ConsumeToText(u8"chapter -3 section", ConsumeCounterReset) == "chapter -3 section 0");
    CHECK(ConsumeToText(u8"a calc(1 + 2)", ConsumeCounterReset) == "a calc(3)");

    CHECK(ConsumeToText(u8"3", ConsumeCounterReset) == "<null>");
    CHECK(ConsumeToText(u8"chapter 1.5", ConsumeCounterReset) == "<null>");
    CHECK(ConsumeToText(u8"chapter, section", ConsumeCounterReset) == "<null>");
    CHECK(ConsumeToText(u8"inherit", ConsumeCounterReset) == "<null>");
    CHECK(ConsumeToText(u8"none chapter", ConsumeCounterReset) == "<partial>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeCounterIncrement and ConsumeCounterSet)",
            "[CSS][Properties][Consumers]")
  {
    // counter-increment defaults to 1; counter-set to 0.
    CHECK(ConsumeToText(u8"chapter", ConsumeCounterIncrement) == "chapter 1");
    CHECK(ConsumeToText(u8"chapter 2 section", ConsumeCounterIncrement) == "chapter 2 section 1");
    CHECK(ConsumeToText(u8"none", ConsumeCounterIncrement) == "none");

    CHECK(ConsumeToText(u8"chapter", ConsumeCounterSet) == "chapter 0");
    CHECK(ConsumeToText(u8"chapter 5", ConsumeCounterSet) == "chapter 5");
    CHECK(ConsumeToText(u8"none", ConsumeCounterSet) == "none");
  }
}

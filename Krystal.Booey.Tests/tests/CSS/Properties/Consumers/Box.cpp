#include "Krystal.Booey/CSS/Properties/Consumers/Box.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeMarginTrim)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeMarginTrim) == "none");
    CHECK(ConsumeToText(u8"block", ConsumeMarginTrim) == "block");
    CHECK(ConsumeToText(u8"inline block", ConsumeMarginTrim) == "inline block");
    CHECK(ConsumeToText(u8"block-start", ConsumeMarginTrim) == "block-start");
    CHECK(ConsumeToText(u8"inline-end block-start", ConsumeMarginTrim) == "inline-end block-start");

    // Pairs and the full set collapse to the shorter forms.
    CHECK(ConsumeToText(u8"block-end block-start", ConsumeMarginTrim) == "block");
    CHECK(ConsumeToText(u8"inline-start inline-end", ConsumeMarginTrim) == "inline");
    CHECK(ConsumeToText(u8"block-start inline-start block-end inline-end", ConsumeMarginTrim)
          == "block inline");

    CHECK(ConsumeToText(u8"block block", ConsumeMarginTrim) == "<null>");
    CHECK(ConsumeToText(u8"block-start block-start", ConsumeMarginTrim) == "<null>");
    CHECK(ConsumeToText(u8"block block-start", ConsumeMarginTrim) == "<partial>");
  }
}

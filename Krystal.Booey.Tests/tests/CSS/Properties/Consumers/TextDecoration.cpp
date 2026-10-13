#include "Krystal.Booey/CSS/Properties/Consumers/TextDecoration.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeTextShadow)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeTextShadow) == "none");
    CHECK(ConsumeToText(u8"1px 2px", ConsumeTextShadow) == "1px 2px");
    CHECK(ConsumeToText(u8"1px 2px 3px", ConsumeTextShadow) == "1px 2px 3px");

    // The colour may come before or after the lengths; it's serialised first.
    CHECK(ConsumeToText(u8"red 1px 2px", ConsumeTextShadow) == "red 1px 2px");
    CHECK(ConsumeToText(u8"1px 2px 3px blue", ConsumeTextShadow) == "blue 1px 2px 3px");
    CHECK(ConsumeToText(u8"1px 2px red, 0 0 5px", ConsumeTextShadow) == "red 1px 2px, 0px 0px 5px");

    CHECK(ConsumeToText(u8"1px", ConsumeTextShadow) == "<null>");
    CHECK(ConsumeToText(u8"red", ConsumeTextShadow) == "<null>");
    CHECK(ConsumeToText(u8"red blue 1px 2px", ConsumeTextShadow) == "<null>");
    CHECK(ConsumeToText(u8"1px 2px, ", ConsumeTextShadow) == "<null>");
    CHECK(ConsumeToText(u8"1px 2px red 3px", ConsumeTextShadow) == "<partial>");

    // The blur radius can't be negative; the shadow ends before it.
    CHECK(ConsumeToText(u8"1px 2px -3px", ConsumeTextShadow) == "<partial>");
  }
}

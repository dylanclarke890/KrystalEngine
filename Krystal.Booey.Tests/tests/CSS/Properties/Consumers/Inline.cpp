#include "Krystal.Booey/CSS/Properties/Consumers/Inline.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeLineFitEdge)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"leading", ConsumeLineFitEdge) == "leading");
    CHECK(ConsumeToText(u8"text", ConsumeLineFitEdge) == "text");
    CHECK(ConsumeToText(u8"cap alphabetic", ConsumeLineFitEdge) == "cap alphabetic");
    CHECK(ConsumeToText(u8"ideographic ideographic-ink", ConsumeLineFitEdge)
          == "ideographic ideographic-ink");

    // The second keyword is dropped when it's the one that would be assumed.
    CHECK(ConsumeToText(u8"text text", ConsumeLineFitEdge) == "text");
    CHECK(ConsumeToText(u8"cap text", ConsumeLineFitEdge) == "cap");
    CHECK(ConsumeToText(u8"ex text", ConsumeLineFitEdge) == "ex");
    CHECK(ConsumeToText(u8"ideographic ideographic", ConsumeLineFitEdge) == "ideographic");

    CHECK(ConsumeToText(u8"alphabetic", ConsumeLineFitEdge) == "<null>");
    CHECK(ConsumeToText(u8"auto", ConsumeLineFitEdge) == "<null>");
    CHECK(ConsumeToText(u8"text cap", ConsumeLineFitEdge) == "<partial>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeTextBoxEdge)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"auto", ConsumeTextBoxEdge) == "auto");
    CHECK(ConsumeToText(u8"cap alphabetic", ConsumeTextBoxEdge) == "cap alphabetic");
    CHECK(ConsumeToText(u8"leading", ConsumeTextBoxEdge) == "<null>");
  }
}

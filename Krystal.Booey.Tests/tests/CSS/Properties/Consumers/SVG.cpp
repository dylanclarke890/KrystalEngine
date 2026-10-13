#include "Krystal.Booey/CSS/Properties/Consumers/SVG.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumePaintOrder)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"normal", ConsumePaintOrder) == "normal");
    CHECK(ConsumeToText(u8"fill", ConsumePaintOrder) == "fill");
    CHECK(ConsumeToText(u8"stroke", ConsumePaintOrder) == "stroke");
    CHECK(ConsumeToText(u8"markers", ConsumePaintOrder) == "markers");

    // Only the keywords needed to recover the order are kept.
    CHECK(ConsumeToText(u8"fill markers", ConsumePaintOrder) == "fill markers");
    CHECK(ConsumeToText(u8"fill stroke markers", ConsumePaintOrder) == "fill");
    CHECK(ConsumeToText(u8"stroke markers fill", ConsumePaintOrder) == "stroke markers");
    CHECK(ConsumeToText(u8"markers stroke", ConsumePaintOrder) == "markers stroke");
    CHECK(ConsumeToText(u8"markers fill stroke", ConsumePaintOrder) == "markers");

    CHECK(ConsumeToText(u8"fill fill", ConsumePaintOrder) == "<null>");
    CHECK(ConsumeToText(u8"fill normal", ConsumePaintOrder) == "<null>");
    CHECK(ConsumeToText(u8"auto", ConsumePaintOrder) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeStrokeDasharray)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeStrokeDasharray) == "none");
    CHECK(ConsumeToText(u8"5px", ConsumeStrokeDasharray) == "5px");
    CHECK(ConsumeToText(u8"5px, 10%", ConsumeStrokeDasharray) == "5px, 10%");
    CHECK(ConsumeToText(u8"1 2", ConsumeStrokeDasharray) == "1, 2");
    CHECK(ConsumeToText(u8"1 2, 3px", ConsumeStrokeDasharray) == "1, 2, 3px");

    // Unitless zero is a number here, not a length.
    CHECK(ConsumeToText(u8"0", ConsumeStrokeDasharray) == "0");

    CHECK(ConsumeToText(u8"-1", ConsumeStrokeDasharray) == "<null>");
    CHECK(ConsumeToText(u8"-1px", ConsumeStrokeDasharray) == "<null>");
    CHECK(ConsumeToText(u8"1,", ConsumeStrokeDasharray) == "<null>");
    CHECK(ConsumeToText(u8"1 none", ConsumeStrokeDasharray) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeStrokeDasharray SVG attribute)", "[CSS][Properties][Consumers]")
  {
    // The length is parsed in standard mode even in an SVG attribute, so a unitless number is a number.
    ParserContext context {ParserMode::SVGAttribute};
    CHECK(ConsumeToText(u8"5 10", ConsumeStrokeDasharray, context) == "5, 10");
  }
}

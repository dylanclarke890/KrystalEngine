#include "Krystal.Booey/CSS/Properties/Consumers/Display.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeDisplay)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeDisplay) == "none");
    CHECK(ConsumeToText(u8"table-cell", ConsumeDisplay) == "table-cell");
    CHECK(ConsumeToText(u8"list-item", ConsumeDisplay) == "list-item");
    CHECK(ConsumeToText(u8"-webkit-box", ConsumeDisplay) == "-webkit-box");

    // Two-value syntax maps to the short values.
    CHECK(ConsumeToText(u8"block", ConsumeDisplay) == "block");
    CHECK(ConsumeToText(u8"flow", ConsumeDisplay) == "block");
    CHECK(ConsumeToText(u8"block flow", ConsumeDisplay) == "block");
    CHECK(ConsumeToText(u8"inline", ConsumeDisplay) == "inline");
    CHECK(ConsumeToText(u8"inline flex", ConsumeDisplay) == "inline-flex");
    CHECK(ConsumeToText(u8"flow-root inline", ConsumeDisplay) == "inline-block");
    CHECK(ConsumeToText(u8"grid", ConsumeDisplay) == "grid");
    CHECK(ConsumeToText(u8"inline table", ConsumeDisplay) == "inline-table");
    CHECK(ConsumeToText(u8"-webkit-inline-flex", ConsumeDisplay) == "inline-flex");
    CHECK(ConsumeToText(u8"-webkit-flex", ConsumeDisplay) == "flex");

    CHECK(ConsumeToText(u8"block inline", ConsumeDisplay) == "<null>");
    CHECK(ConsumeToText(u8"flex grid", ConsumeDisplay) == "<null>");
    CHECK(ConsumeToText(u8"foo", ConsumeDisplay) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeDisplay ruby)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"ruby", ConsumeDisplay) == "<null>");
    CHECK(ConsumeToText(u8"ruby-text", ConsumeDisplay) == "<null>");

    ParserContext context {ParserMode::UASheet};
    CHECK(ConsumeToText(u8"ruby", ConsumeDisplay, context) == "ruby");
    CHECK(ConsumeToText(u8"inline ruby", ConsumeDisplay, context) == "ruby");
    CHECK(ConsumeToText(u8"block ruby", ConsumeDisplay, context) == "block-ruby");
    CHECK(ConsumeToText(u8"ruby-base", ConsumeDisplay, context) == "ruby-base");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeDisplay grid-lanes)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard};
    CHECK(ConsumeToText(u8"grid-lanes", ConsumeDisplay, context) == "<null>");
    CHECK(ConsumeToText(u8"inline-grid-lanes", ConsumeDisplay, context) == "<null>");

    context.gridLanesEnabled = true;
    CHECK(ConsumeToText(u8"grid-lanes", ConsumeDisplay, context) == "grid-lanes");
    CHECK(ConsumeToText(u8"inline grid-lanes", ConsumeDisplay, context) == "inline-grid-lanes");
    CHECK(ConsumeToText(u8"inline-grid-lanes", ConsumeDisplay, context) == "inline-grid-lanes");
  }
}

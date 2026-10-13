#include "Krystal.Booey/CSS/Properties/Consumers/Align.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeAlignContent)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"center", ConsumeAlignContent) == "center");
    CHECK(ConsumeToText(u8"space-between", ConsumeAlignContent) == "space-between");
    CHECK(ConsumeToText(u8"safe center", ConsumeAlignContent) == "safe center");
    CHECK(ConsumeToText(u8"unsafe flex-end", ConsumeAlignContent) == "unsafe flex-end");
    CHECK(ConsumeToText(u8"baseline", ConsumeAlignContent) == "baseline");
    CHECK(ConsumeToText(u8"first baseline", ConsumeAlignContent) == "baseline");
    CHECK(ConsumeToText(u8"last baseline", ConsumeAlignContent) == "last baseline");

    CHECK(ConsumeToText(u8"left", ConsumeAlignContent) == "<null>");
    CHECK(ConsumeToText(u8"safe left", ConsumeAlignContent) == "<null>");
    CHECK(ConsumeToText(u8"first", ConsumeAlignContent) == "<null>");
    CHECK(ConsumeToText(u8"baseline first", ConsumeAlignContent) == "<partial>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeJustifyContent)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"left", ConsumeJustifyContent) == "left");
    CHECK(ConsumeToText(u8"unsafe right", ConsumeJustifyContent) == "unsafe right");
    CHECK(ConsumeToText(u8"stretch", ConsumeJustifyContent) == "stretch");
    CHECK(ConsumeToText(u8"baseline", ConsumeJustifyContent) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeAlignSelf and ConsumeAlignItems)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"auto", ConsumeAlignSelf) == "auto");
    CHECK(ConsumeToText(u8"self-end", ConsumeAlignSelf) == "self-end");
    CHECK(ConsumeToText(u8"safe self-start", ConsumeAlignSelf) == "safe self-start");
    CHECK(ConsumeToText(u8"left", ConsumeAlignSelf) == "<null>");

    CHECK(ConsumeToText(u8"auto", ConsumeAlignItems) == "<null>");
    CHECK(ConsumeToText(u8"normal", ConsumeAlignItems) == "normal");
    CHECK(ConsumeToText(u8"last baseline", ConsumeAlignItems) == "last baseline");

    // anchor-center is behind a setting.
    ParserContext context {ParserMode::HTMLStandard};
    CHECK(ConsumeToText(u8"anchor-center", ConsumeAlignSelf, context) == "<null>");
    CHECK(ConsumeToText(u8"safe anchor-center", ConsumeAlignItems, context) == "<null>");
    context.PropertySettings.cssAnchorPositioningEnabled = true;
    CHECK(ConsumeToText(u8"anchor-center", ConsumeAlignSelf, context) == "anchor-center");
    CHECK(ConsumeToText(u8"safe anchor-center", ConsumeAlignItems, context) == "safe anchor-center");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeJustifySelf and ConsumeJustifyItems)",
            "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"left", ConsumeJustifySelf) == "left");
    CHECK(ConsumeToText(u8"unsafe right", ConsumeJustifySelf) == "unsafe right");
    CHECK(ConsumeToText(u8"auto", ConsumeJustifySelf) == "auto");

    CHECK(ConsumeToText(u8"legacy", ConsumeJustifyItems) == "legacy");
    CHECK(ConsumeToText(u8"legacy left", ConsumeJustifyItems) == "legacy left");
    CHECK(ConsumeToText(u8"center legacy", ConsumeJustifyItems) == "legacy center");
    CHECK(ConsumeToText(u8"right", ConsumeJustifyItems) == "right");
    CHECK(ConsumeToText(u8"self-start", ConsumeJustifyItems) == "self-start");
    CHECK(ConsumeToText(u8"auto", ConsumeJustifyItems) == "<null>");
    CHECK(ConsumeToText(u8"legacy start", ConsumeJustifyItems) == "<partial>");
  }
}

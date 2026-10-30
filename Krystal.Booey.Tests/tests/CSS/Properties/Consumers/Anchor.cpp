#include "Krystal.Booey/CSS/Properties/Consumers/Anchor.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;
  using css::tests::CssText;
  using css::tests::ValueFixture;

  TEST_CASE("PropertyParserHelpers(ConsumePositionArea)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumePositionArea) == "none");
    CHECK(ConsumeToText(u8"top", ConsumePositionArea) == "top");
    CHECK(ConsumeToText(u8"left top", ConsumePositionArea) == "left top");

    // The X/block keyword goes first; span-all next to an explicit keyword is dropped.
    CHECK(ConsumeToText(u8"top left", ConsumePositionArea) == "left top");
    CHECK(ConsumeToText(u8"inline-end block-start", ConsumePositionArea) == "block-start inline-end");
    CHECK(ConsumeToText(u8"span-all top", ConsumePositionArea) == "top");
    CHECK(ConsumeToText(u8"start span-end", ConsumePositionArea) == "start span-end");
    CHECK(ConsumeToText(u8"center center", ConsumePositionArea) == "center");

    CHECK(ConsumeToText(u8"left right", ConsumePositionArea) == "<null>");
    CHECK(ConsumeToText(u8"top start", ConsumePositionArea) == "<null>");
    CHECK(ConsumeToText(u8"foo", ConsumePositionArea) == "<null>");
    CHECK(ConsumeToText(u8"5", ConsumePositionArea) == "<null>");
  }

  TEST_CASE_METHOD(ValueFixture, "PropertyParserHelpers(ValueForPositionArea)",
                   "[CSS][Properties][Consumers]")
  {
    auto specified = ValueForPositionArea(ValueId::BlockStart, ValueId::InlineEnd, ValueType::Specified);
    REQUIRE(specified);
    CHECK(CssText(*specified) == "block-start inline-end");

    // Computed values drop the explicit axis when both keywords are logical.
    auto computed = ValueForPositionArea(ValueId::InlineEnd, ValueId::BlockStart, ValueType::Computed);
    REQUIRE(computed);
    CHECK(CssText(*computed) == "start end");

    auto axisless = ValueForPositionArea(ValueId::Center, ValueId::SpanSelfInlineStart, ValueType::Computed);
    REQUIRE(axisless);
    CHECK(CssText(*axisless) == "center span-self-start");

    // Physical keywords keep their axis.
    auto physical = ValueForPositionArea(ValueId::Top, ValueId::Left, ValueType::Computed);
    REQUIRE(physical);
    CHECK(CssText(*physical) == "left top");

    CHECK_FALSE(ValueForPositionArea(ValueId::Auto, ValueId::Top, ValueType::Specified));
  }
}

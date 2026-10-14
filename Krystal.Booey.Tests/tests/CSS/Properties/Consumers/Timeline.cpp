#include "Krystal.Booey/CSS/Properties/Consumers/Timeline.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;
  using css::tests::CssText;

  TEST_CASE("PropertyParserHelpers(ConsumeAnimationTimelineScroll)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"scroll()", ConsumeAnimationTimelineScroll) == "scroll()");
    CHECK(ConsumeToText(u8"scroll(root)", ConsumeAnimationTimelineScroll) == "scroll(root)");
    CHECK(ConsumeToText(u8"scroll(self x)", ConsumeAnimationTimelineScroll) == "scroll(self x)");

    // The default scroller and axis are omitted.
    CHECK(ConsumeToText(u8"scroll(inline nearest)", ConsumeAnimationTimelineScroll) == "scroll(inline)");

    CHECK(ConsumeToText(u8"scroll(root root)", ConsumeAnimationTimelineScroll) == "<null>");
    CHECK(ConsumeToText(u8"scroll(1px)", ConsumeAnimationTimelineScroll) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeAnimationTimelineView)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"view()", ConsumeAnimationTimelineView) == "view()");
    CHECK(ConsumeToText(u8"view(x)", ConsumeAnimationTimelineView) == "view(x)");
    CHECK(ConsumeToText(u8"view(10px)", ConsumeAnimationTimelineView) == "view(10px)");
    CHECK(ConsumeToText(u8"view(block 10px 20%)", ConsumeAnimationTimelineView) == "view(10px 20%)");
    CHECK(ConsumeToText(u8"view(auto 5px x)", ConsumeAnimationTimelineView) == "view(x auto 5px)");

    CHECK(ConsumeToText(u8"view(x x)", ConsumeAnimationTimelineView) == "<null>");
    CHECK(ConsumeToText(u8"view(red)", ConsumeAnimationTimelineView) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeSingleViewTimelineInsetItem)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"auto", ConsumeSingleViewTimelineInsetItem) == "auto");
    CHECK(ConsumeToText(u8"10px", ConsumeSingleViewTimelineInsetItem) == "10px");
    CHECK(ConsumeToText(u8"10px 10px", ConsumeSingleViewTimelineInsetItem) == "10px");
    CHECK(ConsumeToText(u8"10% auto", ConsumeSingleViewTimelineInsetItem) == "10% auto");
    CHECK(ConsumeToText(u8"red", ConsumeSingleViewTimelineInsetItem) == "<null>");

    ParserContext context {ParserMode::HTMLStandard};
    auto value = ParseSingleViewTimelineInsetItem(CSSOMString(u8" 10px 20% "), context);
    REQUIRE(value);
    CHECK(CssText(*value) == "10px 20%");
    CHECK_FALSE(ParseSingleViewTimelineInsetItem(CSSOMString(u8"10px red"), context));
  }

  TEST_CASE("PropertyParserHelpers(ConsumeSingleAnimationRange)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"normal", ConsumeSingleAnimationRangeStart) == "normal");
    CHECK(ConsumeToText(u8"cover", ConsumeSingleAnimationRangeStart) == "cover");
    CHECK(ConsumeToText(u8"contain 10%", ConsumeSingleAnimationRangeStart) == "contain 10%");
    CHECK(ConsumeToText(u8"20px", ConsumeSingleAnimationRangeStart) == "20px");
    CHECK(ConsumeToText(u8"exit-crossing", ConsumeSingleAnimationRangeStart) == "exit-crossing");

    // The default offset (0% for the start, 100% for the end) is omitted.
    CHECK(ConsumeToText(u8"entry 0%", ConsumeSingleAnimationRangeStart) == "entry");
    CHECK(ConsumeToText(u8"exit 100%", ConsumeSingleAnimationRangeEnd) == "exit");
    CHECK(ConsumeToText(u8"entry 100%", ConsumeSingleAnimationRangeEnd) == "entry");
    CHECK(ConsumeToText(u8"50%", ConsumeSingleAnimationRangeEnd) == "50%");

    CHECK(ConsumeToText(u8"cover red", ConsumeSingleAnimationRangeStart) == "<partial>");
    CHECK(ConsumeToText(u8"red", ConsumeSingleAnimationRangeStart) == "<null>");

    ParserContext context {ParserMode::HTMLStandard};
    auto value =
      ParseSingleAnimationRange(CSSOMString(u8" cover 10% "), context, SingleAnimationRangeType::Start);
    REQUIRE(value);
    CHECK(CssText(*value) == "cover 10%");
    CHECK_FALSE(
      ParseSingleAnimationRange(CSSOMString(u8"cover red"), context, SingleAnimationRangeType::End));
  }

  TEST_CASE("PropertyParserHelpers(IsAnimationRangeKeyword)", "[CSS][Properties][Consumers]")
  {
    CHECK(IsAnimationRangeKeyword(ValueId::Cover));
    CHECK(IsAnimationRangeKeyword(ValueId::EntryCrossing));
    CHECK(IsAnimationRangeKeyword(ValueId::Normal));
    CHECK_FALSE(IsAnimationRangeKeyword(ValueId::Auto));
  }
}

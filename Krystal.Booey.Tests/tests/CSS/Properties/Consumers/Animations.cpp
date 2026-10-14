#include "Krystal.Booey/CSS/Properties/Consumers/Animations.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeKeyframesName)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"foo", ConsumeKeyframesName) == "foo");
    CHECK(ConsumeToText(u8"none", ConsumeKeyframesName) == "none");

    // A string that is a valid <custom-ident> becomes one; `none` stays a string.
    CHECK(ConsumeToText(u8"\"foo\"", ConsumeKeyframesName) == "foo");
    CHECK(ConsumeToText(u8"\"none\"", ConsumeKeyframesName) == "\"none\"");

    // As upstream: the empty string isn't a keyword, so it becomes an empty <custom-ident>.
    CHECK(ConsumeToText(u8"\"\"", ConsumeKeyframesName).empty());

    CHECK(ConsumeToText(u8"initial", ConsumeKeyframesName) == "<null>");
    CHECK(ConsumeToText(u8"10px", ConsumeKeyframesName) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ParseKeyframeKeyList)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard};

    auto keys = ParseKeyframeKeyList(CSSOMString(u8" from, 50%, to "), context);
    REQUIRE(keys.size() == 3uz);
    CHECK(keys[0] == Pair<ValueId, double> {ValueId::Normal, 0.0});
    CHECK(keys[1] == Pair<ValueId, double> {ValueId::Normal, 0.5});
    CHECK(keys[2] == Pair<ValueId, double> {ValueId::Normal, 1.0});

    // A timeline range name with an offset.
    keys = ParseKeyframeKeyList(CSSOMString(u8"cover 25%"), context);
    REQUIRE(keys.size() == 1uz);
    CHECK(keys[0] == Pair<ValueId, double> {ValueId::Cover, 0.25});

    CHECK(ParseKeyframeKeyList(CSSOMString(u8"150%"), context).empty());
    CHECK(ParseKeyframeKeyList(CSSOMString(u8"from to"), context).empty());
    CHECK(ParseKeyframeKeyList(CSSOMString(u8"middle"), context).empty());
    CHECK(ParseKeyframeKeyList(CSSOMString(u8"from,"), context).empty());
  }
}

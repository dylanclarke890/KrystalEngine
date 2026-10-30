#include "Krystal.Booey/CSS/Properties/Consumers/Motion.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeOffsetPath)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard, url::URL(u8"https://example.com/dir/")};

    CHECK(ConsumeToText(u8"none", ConsumeOffsetPath) == "none");
    CHECK(ConsumeToText(u8"url(#p)", ConsumeOffsetPath, context) == "url(\"#p\")");
    CHECK(ConsumeToText(u8"border-box", ConsumeOffsetPath) == "border-box");
    CHECK(ConsumeToText(u8"circle(5px) content-box", ConsumeOffsetPath) == "circle(5px) content-box");
    CHECK(ConsumeToText(u8"path(\"M0 0 L10 10\")", ConsumeOffsetPath) == "path(\"M 0 0 L 10 10\")");

    // ray(): `closest-side` is the default size, so it's omitted.
    CHECK(ConsumeToText(u8"ray(45deg)", ConsumeOffsetPath) == "ray(45deg)");
    CHECK(ConsumeToText(u8"ray(45deg closest-side)", ConsumeOffsetPath) == "ray(45deg)");
    CHECK(ConsumeToText(u8"ray(45deg farthest-corner)", ConsumeOffsetPath) == "ray(45deg farthest-corner)");
    CHECK(ConsumeToText(u8"ray(contain 10deg)", ConsumeOffsetPath) == "ray(10deg contain)");

    CHECK(ConsumeToText(u8"ray(45deg closest-side closest-side)", ConsumeOffsetPath) == "<null>");
    CHECK(ConsumeToText(u8"ray()", ConsumeOffsetPath) == "<null>");
  }
}

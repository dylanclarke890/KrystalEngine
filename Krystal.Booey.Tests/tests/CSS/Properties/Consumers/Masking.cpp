#include "Krystal.Booey/CSS/Properties/Consumers/Masking.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeClipRectFunction)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"rect(1px, 2px, 3px, 4px)", ConsumeClipRectFunction) == "rect(1px, 2px, 3px, 4px)");
    CHECK(ConsumeToText(u8"rect(auto, 2px, auto, 4px)", ConsumeClipRectFunction)
          == "rect(auto, 2px, auto, 4px)");

    // The legacy space-separated form, serialised with commas.
    CHECK(ConsumeToText(u8"rect(1px 2px 3px 4px)", ConsumeClipRectFunction) == "rect(1px, 2px, 3px, 4px)");

    // The two forms can't be mixed.
    CHECK(ConsumeToText(u8"rect(1px, 2px 3px, 4px)", ConsumeClipRectFunction) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeClipPath)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard, url::URL(u8"https://example.com/dir/")};

    CHECK(ConsumeToText(u8"none", ConsumeClipPath) == "none");
    CHECK(ConsumeToText(u8"url(#a)", ConsumeClipPath, context) == "url(\"#a\")");
    CHECK(ConsumeToText(u8"circle(10px)", ConsumeClipPath) == "circle(10px)");
    CHECK(ConsumeToText(u8"margin-box", ConsumeClipPath) == "margin-box");
    CHECK(ConsumeToText(u8"polygon(0 0, 10px 0, 0 10px)", ConsumeClipPath)
          == "polygon(0px 0px, 10px 0px, 0px 10px)");

    // `border-box` is the default reference box, so it's omitted.
    CHECK(ConsumeToText(u8"inset(1px) border-box", ConsumeClipPath) == "inset(1px)");
    CHECK(ConsumeToText(u8"inset(1px) content-box", ConsumeClipPath) == "inset(1px) content-box");

    CHECK(ConsumeToText(u8"red", ConsumeClipPath) == "<null>");
  }
}

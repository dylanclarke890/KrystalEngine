#include "Krystal.Booey/CSS/Properties/Consumers/Filter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeFilter)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard, url::URL(u8"https://example.com/dir/")};

    CHECK(ConsumeToText(u8"none", ConsumeFilter) == "none");
    CHECK(ConsumeToText(u8"blur()", ConsumeFilter) == "blur()");
    CHECK(ConsumeToText(u8"blur(2px)", ConsumeFilter) == "blur(2px)");
    CHECK(ConsumeToText(u8"brightness(50%) contrast(2)", ConsumeFilter) == "brightness(50%) contrast(2)");
    CHECK(ConsumeToText(u8"saturate(300%)", ConsumeFilter) == "saturate(300%)");
    CHECK(ConsumeToText(u8"hue-rotate(0)", ConsumeFilter) == "hue-rotate(0deg)");
    CHECK(ConsumeToText(u8"url(f.svg#a) invert()", ConsumeFilter, context) == "url(\"f.svg#a\") invert()");

    // Values that can't exceed one are clamped.
    CHECK(ConsumeToText(u8"grayscale(2)", ConsumeFilter) == "grayscale(1)");
    CHECK(ConsumeToText(u8"opacity(150%)", ConsumeFilter) == "opacity(100%)");
    CHECK(ConsumeToText(u8"sepia(0.5) invert(1)", ConsumeFilter) == "sepia(0.5) invert(1)");

    CHECK(ConsumeToText(u8"drop-shadow(1px 2px)", ConsumeFilter) == "drop-shadow(1px 2px)");
    CHECK(ConsumeToText(u8"drop-shadow(1px 2px 3px red)", ConsumeFilter) == "drop-shadow(red 1px 2px 3px)");
    CHECK(ConsumeToText(u8"drop-shadow(red 1px 2px)", ConsumeFilter) == "drop-shadow(red 1px 2px)");

    CHECK(ConsumeToText(u8"blur(-1px)", ConsumeFilter) == "<null>");
    CHECK(ConsumeToText(u8"blur(1px 2px)", ConsumeFilter) == "<null>");
    CHECK(ConsumeToText(u8"brightness(-1)", ConsumeFilter) == "<null>");
    CHECK(ConsumeToText(u8"hue-rotate(1)", ConsumeFilter) == "<null>");
    CHECK(ConsumeToText(u8"drop-shadow(1px)", ConsumeFilter) == "<null>");
    CHECK(ConsumeToText(u8"drop-shadow(1px 2px -3px)", ConsumeFilter) == "<null>");
    CHECK(ConsumeToText(u8"drop-shadow(red 1px 2px blue)", ConsumeFilter) == "<null>");
    CHECK(ConsumeToText(u8"blur(1px) none", ConsumeFilter) == "<null>");
    CHECK(ConsumeToText(u8"apple-invert-lightness()", ConsumeFilter) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeAppleColorFilter)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeAppleColorFilter) == "none");
    CHECK(ConsumeToText(u8"apple-invert-lightness()", ConsumeAppleColorFilter) == "apple-invert-lightness()");
    CHECK(ConsumeToText(u8"apple-invert-lightness() sepia(50%)", ConsumeAppleColorFilter)
          == "apple-invert-lightness() sepia(50%)");

    CHECK(ConsumeToText(u8"apple-invert-lightness(1)", ConsumeAppleColorFilter) == "<null>");
    CHECK(ConsumeToText(u8"-apple-invert-lightness()", ConsumeAppleColorFilter) == "<null>");
    CHECK(ConsumeToText(u8"blur(1px)", ConsumeAppleColorFilter) == "<null>");
    CHECK(ConsumeToText(u8"drop-shadow(1px 2px)", ConsumeAppleColorFilter) == "<null>");
    CHECK(ConsumeToText(u8"url(#a)", ConsumeAppleColorFilter) == "<null>");
  }
}

#include "Krystal.Booey/CSS/Parser/ParserFastPaths.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("ParserFastPaths::ParseHexColor", "[CSS][Parser][FastPaths]")
  {
    CHECK(ParserFastPaths::ParseHexColor(u8"f80") == SRGBA<uint8> {255, 136, 0});
    CHECK(ParserFastPaths::ParseHexColor(u8"f808") == SRGBA<uint8> {255, 136, 0, 136});
    CHECK(ParserFastPaths::ParseHexColor(u8"FF8000") == SRGBA<uint8> {255, 128, 0});
    CHECK(ParserFastPaths::ParseHexColor(u8"ff800080") == SRGBA<uint8> {255, 128, 0, 128});
    CHECK_FALSE(ParserFastPaths::ParseHexColor(u8""));
    CHECK_FALSE(ParserFastPaths::ParseHexColor(u8"ff"));
    CHECK_FALSE(ParserFastPaths::ParseHexColor(u8"ff800"));
    CHECK_FALSE(ParserFastPaths::ParseHexColor(u8"#ff8000"));
    CHECK_FALSE(ParserFastPaths::ParseHexColor(u8"ggg"));
  }

  TEST_CASE("ParserFastPaths::ParseNamedColor", "[CSS][Parser][FastPaths]")
  {
    CHECK(ParserFastPaths::ParseNamedColor(u8"red") == SRGBA<uint8> {255, 0, 0});
    CHECK(ParserFastPaths::ParseNamedColor(u8"ReBeccaPurple") == SRGBA<uint8> {102, 51, 153});
    CHECK(ParserFastPaths::ParseNamedColor(u8"transparent") == SRGBA<uint8> {0, 0, 0, 0});
    CHECK_FALSE(ParserFastPaths::ParseNamedColor(u8"currentcolor"));
    CHECK_FALSE(ParserFastPaths::ParseNamedColor(u8"réd"));
    CHECK_FALSE(ParserFastPaths::ParseNamedColor(
      u8"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  }

  TEST_CASE("ParserFastPaths::ParseSimpleColor", "[CSS][Parser][FastPaths]")
  {
    ParserContext standard;

    SECTION("Hex and named colours")
    {
      CHECK(ParserFastPaths::ParseSimpleColor(u8"#f00", standard) == SRGBA<uint8> {255, 0, 0});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"Lime", standard) == SRGBA<uint8> {0, 255, 0});
      CHECK_FALSE(ParserFastPaths::ParseSimpleColor(u8"#f0", standard));

      // Hex without `#` is quirks mode only.
      CHECK_FALSE(ParserFastPaths::ParseSimpleColor(u8"f00", standard));
      ParserContext quirks {.Mode = ParserMode::HTMLQuirks};
      CHECK(ParserFastPaths::ParseSimpleColor(u8"f00", quirks) == SRGBA<uint8> {255, 0, 0});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"00ff00", quirks) == SRGBA<uint8> {0, 255, 0});
      CHECK_FALSE(ParserFastPaths::ParseSimpleColor(u8"f00f", quirks));
    }

    SECTION("rgb() and rgba()")
    {
      CHECK(ParserFastPaths::ParseSimpleColor(u8"rgb(255,128,0)", standard) == SRGBA<uint8> {255, 128, 0});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"RGB( 255 , 128 , 0 )", standard)
            == SRGBA<uint8> {255, 128, 0});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"rgb(100%, 50%, 0%)", standard)
            == SRGBA<uint8> {255, 128, 0});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"rgb(12.5%, 0%, 0%)", standard) == SRGBA<uint8> {32, 0, 0});

      // Clamped.
      CHECK(ParserFastPaths::ParseSimpleColor(u8"rgb(300, -5, 0)", standard) == SRGBA<uint8> {255, 0, 0});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"rgb(200%, 0%, 0%)", standard) == SRGBA<uint8> {255, 0, 0});

      // Alpha.
      CHECK(ParserFastPaths::ParseSimpleColor(u8"rgba(255, 0, 0, 0.5)", standard)
            == SRGBA<uint8> {255, 0, 0, 128});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"rgba(255, 0, 0, .5)", standard)
            == SRGBA<uint8> {255, 0, 0, 128});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"rgba(255, 0, 0, 0.55)", standard)
            == SRGBA<uint8> {255, 0, 0, 140});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"rgba(255, 0, 0, 1)", standard)
            == SRGBA<uint8> {255, 0, 0, 255});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"rgba(255, 0, 0, 0)", standard)
            == SRGBA<uint8> {255, 0, 0, 0});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"rgba(255, 0, 0, 5)", standard)
            == SRGBA<uint8> {255, 0, 0, 255});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"rgba(255, 0, 0, -0.5)", standard)
            == SRGBA<uint8> {255, 0, 0, 0});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"rgb(255, 0, 0, 0.5)", standard)
            == SRGBA<uint8> {255, 0, 0, 128});

      // Numbers and percentages can't be mixed, and only the legacy syntax has a fast path.
      CHECK_FALSE(ParserFastPaths::ParseSimpleColor(u8"rgb(255, 50%, 0)", standard));
      CHECK_FALSE(ParserFastPaths::ParseSimpleColor(u8"rgb(100%, 50, 0)", standard));
      CHECK_FALSE(ParserFastPaths::ParseSimpleColor(u8"rgb(255 0 0)", standard));
      CHECK_FALSE(ParserFastPaths::ParseSimpleColor(u8"rgb(255, 0, 0", standard));
      CHECK_FALSE(ParserFastPaths::ParseSimpleColor(u8"rgb(255, 0, 0) ", standard));
      CHECK_FALSE(ParserFastPaths::ParseSimpleColor(u8"rgb(1.5, 0, 0)", standard));
      CHECK_FALSE(ParserFastPaths::ParseSimpleColor(u8"rgba(255, 0, 0, 0.)", standard));
    }

    SECTION("hsl() and hsla()")
    {
      CHECK(ParserFastPaths::ParseSimpleColor(u8"hsl(120, 100%, 50%)", standard) == SRGBA<uint8> {0, 255, 0});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"hsl(120deg, 100%, 50%)", standard)
            == SRGBA<uint8> {0, 255, 0});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"hsla(0, 100%, 50%, 0.5)", standard)
            == SRGBA<uint8> {255, 0, 0, 128});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"hsl(0, 100%, 50%, 50%)", standard)
            == SRGBA<uint8> {255, 0, 0, 128});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"hsl(3.14159rad, 100%, 50%)", standard)
            == SRGBA<uint8> {0, 255, 255});

      // Only deg/rad (or unitless) hues, and percentages for saturation and lightness.
      CHECK_FALSE(ParserFastPaths::ParseSimpleColor(u8"hsl(0.5turn, 100%, 50%)", standard));
      CHECK_FALSE(ParserFastPaths::ParseSimpleColor(u8"hsl(120, 100, 50)", standard));
      CHECK_FALSE(ParserFastPaths::ParseSimpleColor(u8"hsl(120 100% 50%)", standard));
      CHECK_FALSE(ParserFastPaths::ParseSimpleColor(u8"hsl(120., 100%, 50%)", standard));
    }
  }

  TEST_CASE("ParserFastPaths::LengthValueRangeForPropertiesSupportingSimpleLengths",
            "[CSS][Parser][FastPaths]")
  {
    CHECK(ParserFastPaths::LengthValueRangeForPropertiesSupportingSimpleLengths(PropertyId::Width)
          == NonNegative);
    CHECK(ParserFastPaths::LengthValueRangeForPropertiesSupportingSimpleLengths(PropertyId::ShapeMargin)
          == NonNegative);
    CHECK(ParserFastPaths::LengthValueRangeForPropertiesSupportingSimpleLengths(PropertyId::Top) == All);
    CHECK(ParserFastPaths::LengthValueRangeForPropertiesSupportingSimpleLengths(PropertyId::MarginInlineStart)
          == All);
    CHECK_FALSE(ParserFastPaths::LengthValueRangeForPropertiesSupportingSimpleLengths(PropertyId::Color));
  }
}

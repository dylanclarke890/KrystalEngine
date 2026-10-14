#include "Krystal.Booey/CSS/Parser/ParserFastPaths.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
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
    ParserContext standard {ParserMode::HTMLStandard};

    SECTION("Hex and named colours")
    {
      CHECK(ParserFastPaths::ParseSimpleColor(u8"#f00", standard) == SRGBA<uint8> {255, 0, 0});
      CHECK(ParserFastPaths::ParseSimpleColor(u8"Lime", standard) == SRGBA<uint8> {0, 255, 0});
      CHECK_FALSE(ParserFastPaths::ParseSimpleColor(u8"#f0", standard));

      // Hex without `#` is quirks mode only.
      CHECK_FALSE(ParserFastPaths::ParseSimpleColor(u8"f00", standard));
      ParserContext quirks {ParserMode::HTMLQuirks};
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

  TEST_CASE_METHOD(ValueFixture, "ParserFastPaths::MaybeParseValue", "[CSS][Parser][FastPaths]")
  {
    ParserContext context {ParserMode::HTMLStandard};
    auto parse = [&](PropertyId property, utf8_stringview css, RuleType rule = RuleType::Style)
    {
      PropertyParserState state {.Context = context, .CurrentRule = rule, .CurrentProperty = property};
      auto value = ParserFastPaths::MaybeParseValue(property, css, state);
      return value ? CssText(*value) : "<null>";
    };

    SECTION("display")
    {
      CHECK(parse(PropertyId::Display, u8"block") == "block");
      CHECK(parse(PropertyId::Display, u8"INLINE-FLEX") == "inline-flex");
      CHECK(parse(PropertyId::Display, u8"inherit") == "inherit");
      CHECK(parse(PropertyId::Display, u8"flow") == "<null>");
      CHECK(parse(PropertyId::Display, u8"block flow") == "<null>");
    }

    SECTION("opacity")
    {
      CHECK(parse(PropertyId::Opacity, u8"0.5") == "0.5");
      CHECK(parse(PropertyId::Opacity, u8"50%") == "0.5");
      CHECK(parse(PropertyId::Opacity, u8"5.") == "<null>");
      CHECK(parse(PropertyId::Opacity, u8"inherit") == "<null>");
    }

    SECTION("transform")
    {
      CHECK(parse(PropertyId::Transform, u8"translate(1px,2px)") == "translate(1px, 2px)");
      CHECK(parse(PropertyId::Transform, u8"translateX(0)") == "translateX(0px)");
      CHECK(parse(PropertyId::Transform, u8"translate3d(1px,2%,3px)") == "translate3d(1px, 2%, 3px)");
      CHECK(parse(PropertyId::Transform, u8" ROTATE(45deg) scale3d(1,2,3) ")
            == "rotate(45deg) scale3d(1, 2, 3)");
      CHECK(parse(PropertyId::Transform, u8"rotateZ(1rad)") == "rotateZ(1rad)");
      CHECK(parse(PropertyId::Transform, u8"matrix3d(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1)")
            == "matrix3d(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)");

      CHECK(parse(PropertyId::Transform, u8"translateZ(10%)") == "<null>");
      CHECK(parse(PropertyId::Transform, u8"translate3d(1px,2px,3%)") == "<null>");
      CHECK(parse(PropertyId::Transform, u8"translate(1)") == "<null>");
      CHECK(parse(PropertyId::Transform, u8"rotate(45)") == "<null>");
      CHECK(parse(PropertyId::Transform, u8"rotate(45deg") == "<null>");
      CHECK(parse(PropertyId::Transform, u8"skew(1deg)") == "<null>");
      CHECK(parse(PropertyId::Transform, u8"none") == "<null>");
    }

    SECTION("Colours")
    {
      CHECK(parse(PropertyId::Color, u8"red") == "red");
      CHECK(parse(PropertyId::Color, u8"#ff0000") == "rgb(255, 0, 0)");
      CHECK(parse(PropertyId::Color, u8"rgb(1, 2, 3)") == "rgb(1, 2, 3)");
      CHECK(parse(PropertyId::BackgroundColor, u8"currentcolor") == "currentcolor");
      CHECK(parse(PropertyId::CaretColor, u8"auto") == "auto");

      // `accent-color` only takes `auto` here while it's exposed.
      CHECK(parse(PropertyId::AccentColor, u8"auto") == "<null>");
      context.PropertySettings.accentColorEnabled = true;
      CHECK(parse(PropertyId::AccentColor, u8"auto") == "auto");
      CHECK(parse(PropertyId::Color, u8"10px") == "<null>");

      // Quirky (hashless) colours only apply to the properties that accept them, in quirks mode.
      context.Mode = ParserMode::HTMLQuirks;
      CHECK(parse(PropertyId::Color, u8"ff0000") == "rgb(255, 0, 0)");
      CHECK(parse(PropertyId::BorderLeftColor, u8"ff0000") == "rgb(255, 0, 0)");
      CHECK(parse(PropertyId::OutlineColor, u8"ff0000") == "<null>");
    }

    SECTION("Simple lengths")
    {
      CHECK(parse(PropertyId::Width, u8"10px") == "10px");
      CHECK(parse(PropertyId::Width, u8"10.5PX") == "10.5px");
      CHECK(parse(PropertyId::Width, u8"50%") == "50%");
      CHECK(parse(PropertyId::Width, u8"0") == "0px");
      CHECK(parse(PropertyId::MarginTop, u8"-10px") == "-10px");

      // Unitless lengths are only allowed in SVG attributes; ranges are checked.
      CHECK(parse(PropertyId::Width, u8"10") == "<null>");
      CHECK(parse(PropertyId::Width, u8"-10px") == "<null>");
      context.Mode = ParserMode::SVGAttribute;
      CHECK(parse(PropertyId::Width, u8"10") == "10px");
    }

    SECTION("Keywords")
    {
      CHECK(parse(PropertyId::Width, u8"inherit") == "inherit");
      CHECK(parse(PropertyId::Position, u8"absolute") == "absolute");
      CHECK(parse(PropertyId::Position, u8"ABSOLUTE") == "absolute");
      CHECK(parse(PropertyId::Position, u8"fixed-ish") == "<null>");

      // Shorthands leave CSS-wide keywords to the full parser.
      CHECK(parse(PropertyId::Margin, u8"inherit") == "<null>");
    }

    SECTION("Only style rules use the fast path")
    {
      CHECK(parse(PropertyId::Display, u8"block", RuleType::Keyframe) == "<null>");
    }
  }
}

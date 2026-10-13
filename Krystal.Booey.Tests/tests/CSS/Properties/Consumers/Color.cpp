#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ColorInlines.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ColorInterpolationMethod.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimitMix.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Color/ColorInterpolationMethod.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include <string>

namespace krys::boo::css::tests
{
  using namespace PropertyParserHelpers;

  namespace
  {
    struct ParseOptions
    {
      ParserContext Context {ParserMode::HTMLStandard};
      PropertyId Property {PropertyId::Invalid};
      ColorParsingOptions ColorOptions {};
    };

    /// @brief Tokenizes `css` and consumes a colour from it; null unless every token was consumed.
    Maybe<Color> Parse(utf8_stringview css, const ParseOptions &options = {})
    {
      InputStream input {CSSOMString(css)};
      Tokenizer tokenizer {input};
      tokenizer.PumpTokenizer();
      auto tokens = tokenizer.Tokens();

      PropertyParserState state {.Context = options.Context, .CurrentProperty = options.Property};
      auto result = ConsumeUnresolvedColor(tokens, state, options.ColorOptions);
      if (!tokens.IsAtEnd())
      {
        return null;
      }
      return result;
    }

    /// @brief Serialises as a std::string, which Catch2 can print.
    template <typename T>
    std::string SerialiseForTest(const T &value)
    {
      auto serialised = Serialise(DefaultSerialisationContext(), value);
      return std::string(serialised.begin(), serialised.end());
    }

    std::string Serialised(utf8_stringview css, const ParseOptions &options = {})
    {
      auto color = Parse(css, options);
      return color ? SerialiseForTest(*color) : std::string("<invalid>");
    }

    boo::Color Resolve(const Color &color)
    {
      PlatformColorResolutionState state;
      return CreateColor(color, state);
    }

    /// @brief The colour as 8-bit sRGB, converting out-of-line colours.
    SRGBA<uint8> Bytes(const boo::Color &color)
    {
      return color.ToColorTypeLossy<SRGBA<uint8>>();
    }

    SRGBA<uint8> Bytes(utf8_stringview css, const ParseOptions &options = {})
    {
      auto color = Parse(css, options);
      REQUIRE(color);
      return Bytes(Resolve(*color));
    }

    ParseOptions Quirks(PropertyId property)
    {
      return ParseOptions {.Context = ParserContext {ParserMode::HTMLQuirks}, .Property = property};
    }

    Maybe<DynamicRangeLimit> ParseDynamicRangeLimit(utf8_stringview css, const ParserContext &context)
    {
      InputStream input {CSSOMString(css)};
      Tokenizer tokenizer {input};
      tokenizer.PumpTokenizer();
      auto tokens = tokenizer.Tokens();

      PropertyParserState state {.Context = context};
      auto result = ConsumeUnresolvedDynamicRangeLimit(tokens, state);
      if (!tokens.IsAtEnd())
      {
        return null;
      }
      return result;
    }
  }

  TEST_CASE("Colour parsing(keywords)", "[CSS][Properties][Consumers][Color]")
  {
    auto red = Parse(u8"red");
    REQUIRE(red);
    REQUIRE(red->Keyword());
    CHECK(red->Keyword()->Value == ValueId::Red);

    CHECK(Parse(u8"currentcolor")->Keyword()->Value == ValueId::Currentcolor);
    CHECK(Parse(u8"Canvas")->Keyword()->Value == ValueId::Canvas);
    CHECK_FALSE(Parse(u8"foo"));

    SECTION("Allowed colour types")
    {
      auto absoluteOnly = ParseOptions {.ColorOptions = {.AllowedColorTypes = ColorType::Absolute}};
      CHECK(Parse(u8"red", absoluteOnly));
      CHECK_FALSE(Parse(u8"currentcolor", absoluteOnly));
      CHECK_FALSE(Parse(u8"Canvas", absoluteOnly));
    }

    SECTION("Mode specific keywords")
    {
      CHECK_FALSE(Parse(u8"-webkit-focus-ring-color"));
      CHECK(Parse(u8"-webkit-focus-ring-color", {.Context = ParserContext {ParserMode::HTMLQuirks}}));
      CHECK(Parse(u8"-webkit-focus-ring-color", {.Context = ParserContext {ParserMode::UASheet}}));

      CHECK_FALSE(
        IsColorKeywordAllowed(ValueId::InternalDocumentTextColor, ParserContext {ParserMode::HTMLStandard}));
      CHECK_FALSE(
        IsColorKeywordAllowed(ValueId::InternalDocumentTextColor, ParserContext {ParserMode::HTMLQuirks}));
      CHECK(IsColorKeywordAllowed(ValueId::InternalDocumentTextColor, ParserContext {ParserMode::UASheet}));
      CHECK(IsColorKeywordAllowed(ValueId::Red, ParserContext {ParserMode::HTMLStandard}));
    }
  }

  TEST_CASE("Colour parsing(hex)", "[CSS][Properties][Consumers][Color]")
  {
    auto hex = Parse(u8"#ff8000");
    REQUIRE(hex);
    REQUIRE(hex->Hex());
    CHECK(hex->Hex()->Value == SRGBA<uint8> {255, 128, 0});

    CHECK(Parse(u8"#f80")->Hex()->Value == SRGBA<uint8> {255, 136, 0});
    CHECK(Parse(u8"#f808")->Hex()->Value == SRGBA<uint8> {255, 136, 0, 136});
    CHECK(Parse(u8"#ff800080")->Hex()->Value == SRGBA<uint8> {255, 128, 0, 128});
    CHECK(Parse(u8"#ff80")->Hex()->Value == SRGBA<uint8> {255, 255, 136, 0});
    CHECK_FALSE(Parse(u8"#ff800"));
    CHECK_FALSE(Parse(u8"#ggg"));

    SECTION("Quirky colours")
    {
      // Standards mode, or a property that doesn't accept quirky colours.
      CHECK_FALSE(Parse(u8"ff0000", {.Property = PropertyId::Color}));
      CHECK_FALSE(Parse(u8"ff0000", Quirks(PropertyId::Width)));

      auto quirks = Quirks(PropertyId::Color);
      CHECK(Parse(u8"ff0000", quirks)->Hex()->Value == SRGBA<uint8> {255, 0, 0});
      CHECK(Parse(u8"f00", quirks)->Hex()->Value == SRGBA<uint8> {255, 0, 0});
      CHECK_FALSE(Parse(u8"ff00", quirks));

      // Numbers are zero padded to six digits.
      CHECK(Parse(u8"123456", quirks)->Hex()->Value == SRGBA<uint8> {0x12, 0x34, 0x56});
      CHECK(Parse(u8"1", quirks)->Hex()->Value == SRGBA<uint8> {0, 0, 1});
      CHECK_FALSE(Parse(u8"1234567", quirks));
      CHECK_FALSE(Parse(u8"1.5", quirks));
      CHECK_FALSE(Parse(u8"-1", quirks));

      // Dimensions keep their unit: `0001ff` is the number 1 with the unit `ff`.
      CHECK(Parse(u8"0001ff", quirks)->Hex()->Value == SRGBA<uint8> {0, 0x01, 0xff});
      CHECK(Parse(u8"ff0000", Quirks(PropertyId::BackgroundColor)));

      // Never inside another colour.
      CHECK_FALSE(Parse(u8"light-dark(ff0000, blue)", quirks));
      CHECK(Parse(u8"light-dark(#ff0000, blue)", quirks));
    }
  }

  TEST_CASE("Colour parsing(rgb() and hsl())", "[CSS][Properties][Consumers][Color]")
  {
    SECTION("Modern syntax resolves eagerly")
    {
      auto rgb = Parse(u8"rgb(255 128 0)");
      REQUIRE(rgb);
      REQUIRE(rgb->Resolved());
      CHECK(rgb->Resolved()->Value.TryGetAsSRGBABytes() == SRGBA<uint8> {255, 128, 0});

      CHECK(Serialised(u8"rgb(255 128 0)") == "rgb(255, 128, 0)");
      CHECK(Serialised(u8"rgba(255 128 0 / 50%)") == "rgba(255, 128, 0, 0.5)");
      CHECK(Serialised(u8"rgb(100% 0% 0%)") == "rgb(255, 0, 0)");
      CHECK(Serialised(u8"rgb(none 0 0)") == "rgb(0, 0, 0)");
      CHECK(Serialised(u8"rgb(300 -10 0)") == "rgb(255, 0, 0)");
      CHECK(Serialised(u8"RGB(255 0 0)") == "rgb(255, 0, 0)");
    }

    SECTION("Legacy syntax")
    {
      CHECK(Serialised(u8"rgb(255, 128, 0)") == "rgb(255, 128, 0)");
      CHECK(Serialised(u8"rgba(255, 128, 0, 0.5)") == "rgba(255, 128, 0, 0.5)");
      CHECK(Serialised(u8"rgb(100%, 0%, 0%)") == "rgb(255, 0, 0)");

      // The legacy syntax doesn't allow `none`, mixing numbers and percentages, or a slash.
      CHECK_FALSE(Parse(u8"rgb(none, 0, 0)"));
      CHECK_FALSE(Parse(u8"rgb(255, 0%, 0)"));
      CHECK_FALSE(Parse(u8"rgb(255, 0, 0 / 1)"));
      CHECK_FALSE(Parse(u8"rgb(255, 0 0)"));
      CHECK_FALSE(Parse(u8"rgb(255 0, 0)"));
      CHECK_FALSE(Parse(u8"rgb(255 0 0 0)"));
    }

    SECTION("hsl()")
    {
      CHECK(Serialised(u8"hsl(120deg 100% 50%)") == "rgb(0, 255, 0)");
      CHECK(Serialised(u8"hsl(120 100 50)") == "rgb(0, 255, 0)");
      CHECK(Serialised(u8"hsla(0.5turn 100% 50% / 0.25)") == "rgba(0, 255, 255, 0.25)");
      CHECK(Serialised(u8"hsl(120, 100%, 50%)") == "rgb(0, 255, 0)");
      CHECK(Serialised(u8"hsl(120deg, 100%, 50%, 50%)") == "rgba(0, 255, 0, 0.5)");

      // The legacy syntax only takes percentages for saturation and lightness.
      CHECK_FALSE(Parse(u8"hsl(120, 100, 50)"));
      CHECK_FALSE(Parse(u8"hsl(none, 100%, 50%)"));
    }

    SECTION("calc() that doesn't need conversion data is evaluated eagerly")
    {
      auto rgb = Parse(u8"rgb(calc(200 + 55) 0 0)");
      REQUIRE(rgb);
      CHECK(rgb->IsResolved());
      CHECK(SerialiseForTest(*rgb) == "rgb(255, 0, 0)");
    }

    SECTION("calc() that needs conversion data is kept")
    {
      auto rgb = Parse(u8"rgb(calc(1em / 1px) 0 0)");
      REQUIRE(rgb);
      CHECK_FALSE(rgb->IsResolved());
      CHECK_FALSE(Resolve(*rgb).IsValid());
    }
  }

  TEST_CASE("Colour parsing(other colour functions)", "[CSS][Properties][Consumers][Color]")
  {
    SECTION("lab(), lch(), oklab(), oklch(), hwb()")
    {
      CHECK(Serialised(u8"lab(50% 40 59.5)") == "lab(50 40 59.5)");
      CHECK(Serialised(u8"lab(50 40 59.5 / 0.5)") == "lab(50 40 59.5 / 0.5)");
      CHECK(Serialised(u8"lch(50% 30 270deg)") == "lch(50 30 270)");
      CHECK(Serialised(u8"oklab(0.5 0.1 -0.1)") == "oklab(0.5 0.1 -0.1)");
      CHECK(Serialised(u8"oklch(50% 0.2 90)") == "oklch(0.5 0.2 90)");
      CHECK(Serialised(u8"hwb(120 0% 0%)") == "rgb(0, 255, 0)");
      CHECK(Serialised(u8"lab(none 0 0)") == "lab(none 0 0)");

      // These don't have a legacy syntax.
      CHECK_FALSE(Parse(u8"lab(50, 40, 59.5)"));
      CHECK_FALSE(Parse(u8"hwb(120, 0%, 0%)"));

      // calc() is always kept, and resolves later.
      auto lab = Parse(u8"lab(calc(25 * 2) 0 0)");
      REQUIRE(lab);
      CHECK_FALSE(lab->IsResolved());
      auto resolved = SerialisationForCSS(Resolve(*lab));
      CHECK(std::string(resolved.begin(), resolved.end()) == "lab(50 0 0)");
    }

    SECTION("color()")
    {
      CHECK(Serialised(u8"color(srgb 1 0 0)") == "color(srgb 1 0 0)");
      CHECK(Serialised(u8"color(display-p3 1 0 0 / 50%)") == "color(display-p3 1 0 0 / 0.5)");
      CHECK(Serialised(u8"color(srgb-linear 100% 0% 0%)") == "color(srgb-linear 1 0 0)");
      CHECK(Serialised(u8"color(a98-rgb 1 0 0)") == "color(a98-rgb 1 0 0)");
      CHECK(Serialised(u8"color(prophoto-rgb 1 0 0)") == "color(prophoto-rgb 1 0 0)");
      CHECK(Serialised(u8"color(rec2020 1 0 0)") == "color(rec2020 1 0 0)");
      CHECK(Serialised(u8"color(display-p3-linear 1 0 0)") == "color(display-p3-linear 1 0 0)");
      CHECK(Serialised(u8"color(xyz 1 0 0)") == "color(xyz-d65 1 0 0)");
      CHECK(Serialised(u8"color(xyz-d50 1 0 0)") == "color(xyz-d50 1 0 0)");
      CHECK(Serialised(u8"color(xyz-d65 1 0 0)") == "color(xyz-d65 1 0 0)");
      CHECK_FALSE(Parse(u8"color(foo 1 0 0)"));
      CHECK_FALSE(Parse(u8"color(srgb 1 0)"));
      CHECK_FALSE(Parse(u8"color(srgb, 1, 0, 0)"));
    }

    SECTION("Relative colours")
    {
      auto relative = Parse(u8"rgb(from red r g b)");
      REQUIRE(relative);
      CHECK_FALSE(relative->IsResolved());
      CHECK(SerialiseForTest(*relative) == "rgb(from red r g b)");
      CHECK(Bytes(Resolve(*relative)) == SRGBA<uint8> {255, 0, 0});

      CHECK(Bytes(u8"rgb(from red b g r)") == SRGBA<uint8> {0, 0, 255});
      CHECK(Bytes(u8"rgb(from red r g b / 0.5)") == SRGBA<uint8> {255, 0, 0, 128});
      CHECK(Bytes(u8"rgb(from red calc(r / 2) g b)") == SRGBA<uint8> {128, 0, 0});
      CHECK(Bytes(u8"hsl(from red calc(h + 120) s l)") == SRGBA<uint8> {0, 255, 0});
      CHECK(Bytes(u8"color(from red srgb b g r)") == SRGBA<uint8> {0, 0, 255});
      CHECK(Bytes(u8"lab(from lab(50 0 0) l a b)") == Bytes(u8"lab(50 0 0)"));

      // Channel keywords are only symbols inside a relative colour, and only the function's own.
      CHECK_FALSE(Parse(u8"rgb(r g b)"));
      CHECK_FALSE(Parse(u8"rgb(from red h s l)"));
      CHECK_FALSE(Parse(u8"rgb(from red r g)"));
      CHECK_FALSE(Parse(u8"rgb(from r g b)"));
      CHECK_FALSE(Parse(u8"color(from red r g b)"));
    }

    SECTION("color-mix()")
    {
      CHECK(Serialised(u8"color-mix(in srgb, red, blue)") == "color-mix(in srgb, red, blue)");
      CHECK(Serialised(u8"color-mix(red, blue)") == "color-mix(red, blue)");
      CHECK(Serialised(u8"color-mix(in oklab, red, blue)") == "color-mix(red, blue)");
      CHECK(Serialised(u8"color-mix(in srgb, red 25%, blue)") == "color-mix(in srgb, red 25%, blue)");
      CHECK(Serialised(u8"color-mix(in srgb, 25% red, blue)") == "color-mix(in srgb, red 25%, blue)");
      CHECK(Serialised(u8"color-mix(in hsl longer hue, red, blue)")
            == "color-mix(in hsl longer hue, red, blue)");
      CHECK(Serialised(u8"color-mix(in lch, red, blue)") == "color-mix(in lch, red, blue)");
      CHECK(Bytes(u8"color-mix(in srgb, red, blue)") == SRGBA<uint8> {128, 0, 128});

      CHECK_FALSE(Parse(u8"color-mix(in srgb, red)"));
      CHECK_FALSE(Parse(u8"color-mix(in srgb red, blue)"));
      CHECK_FALSE(Parse(u8"color-mix(in foo, red, blue)"));
      CHECK_FALSE(Parse(u8"color-mix(in hsl longer, red, blue)"));
      CHECK_FALSE(Parse(u8"color-mix(in srgb longer hue, red, blue)"));
      CHECK_FALSE(Parse(u8"color-mix(in srgb, red 25% 25%, blue)"));
      CHECK_FALSE(Parse(u8"color-mix(in srgb, red 101%, blue)"));

      // Both percentages zero.
      CHECK_FALSE(Parse(u8"color-mix(in srgb, red 0%, blue 0%)"));
      CHECK(Parse(u8"color-mix(in srgb, red 0%, blue)"));
      CHECK(Parse(u8"color-mix(in srgb, red calc(0%), blue 0%)"));
    }

    SECTION("light-dark()")
    {
      CHECK(Serialised(u8"light-dark(red, blue)") == "light-dark(red, blue)");
      CHECK_FALSE(Parse(u8"light-dark(red)"));
      CHECK_FALSE(Parse(u8"light-dark(red blue)"));
    }

    SECTION("contrast-color() and color-layers() are behind settings")
    {
      CHECK_FALSE(Parse(u8"contrast-color(red)"));
      CHECK_FALSE(Parse(u8"color-layers(red, blue)"));

      auto enabled = ParseOptions {};
      enabled.Context.colorLayersEnabled = true;
      enabled.Context.contrastColorEnabled = true;
      CHECK(Serialised(u8"contrast-color(red)", enabled) == "contrast-color(red)");
      CHECK(Serialised(u8"color-layers(red, blue)", enabled) == "color-layers(red, blue)");
      CHECK_FALSE(Parse(u8"contrast-color(red, blue)", enabled));
      CHECK_FALSE(Parse(u8"color-layers(red blue)", enabled));
    }

    SECTION("A failed function doesn't consume anything")
    {
      InputStream input {CSSOMString(u8"rgb(foo)")};
      Tokenizer tokenizer {input};
      tokenizer.PumpTokenizer();
      auto tokens = tokenizer.Tokens();
      ParserContext context {ParserMode::HTMLStandard};
      PropertyParserState state {.Context = context};
      CHECK_FALSE(ConsumeUnresolvedColor(tokens, state));
      CHECK(tokens.Peek().Type() == TokenType::Function);
    }
  }

  TEST_CASE("Colour parsing(raw entry points)", "[CSS][Properties][Consumers][Color]")
  {
    ParserContext context {ParserMode::HTMLStandard};

    PlatformColorResolutionState state;
    CHECK(ParseColorRawGeneral(u8"  red ", context, {}, state) == boo::Color::red);
    CHECK(Bytes(ParseColorRawGeneral(u8"rgb(from red b g r)", context, {}, state))
          == SRGBA<uint8> {0, 0, 255});
    CHECK_FALSE(ParseColorRawGeneral(u8"red blue", context, {}, state).IsValid());
    CHECK_FALSE(ParseColorRawGeneral(u8"", context, {}, state).IsValid());
    CHECK_FALSE(ParseColorRawGeneral(u8"   ", context, {}, state).IsValid());
    CHECK_FALSE(DeprecatedParseColorRawWithoutContext(u8"").IsValid());

    CHECK(ParseColorRawSimple(u8"#00ff00", context) == boo::Color::green);
    CHECK_FALSE(ParseColorRawSimple(u8"rgb(0 255 0)", context).IsValid());
    CHECK(ParseColorRaw(u8"rgb(0 255 0)", context) == boo::Color::green);
    CHECK(ParseColorRaw(u8"lime", context) == boo::Color::green);

    CHECK(DeprecatedParseColorRawWithoutContext(u8"blue") == boo::Color::blue);
    CHECK(DeprecatedParseColorRawWithoutContext(u8"hwb(240 0% 0%)") == boo::Color::blue);
    CHECK_FALSE(DeprecatedParseColorRawWithoutContext(u8"currentcolor").IsValid());
    CHECK_FALSE(DeprecatedParseColorRawWithoutContext(u8"blue,").IsValid());

    InputStream input {CSSOMString(u8"color-mix(in srgb, white, black) red")};
    Tokenizer tokenizer {input};
    tokenizer.PumpTokenizer();
    auto tokens = tokenizer.Tokens();
    PropertyParserState propertyState {.Context = context};
    auto mixed = ConsumeColorRaw(tokens, propertyState, {}, state);
    CHECK(Bytes(mixed) == SRGBA<uint8> {128, 128, 128});
    CHECK(tokens.Peek().ValueId() == ValueId::Red);
  }

  TEST_CASE("Colour parsing(<color-interpolation-method>)", "[CSS][Properties][Consumers][Color]")
  {
    auto parse = [](utf8_stringview css) -> Maybe<ColorInterpolationMethod>
    {
      InputStream input {CSSOMString(css)};
      Tokenizer tokenizer {input};
      tokenizer.PumpTokenizer();
      auto tokens = tokenizer.Tokens();
      ParserContext context {ParserMode::HTMLStandard};
      PropertyParserState state {.Context = context};
      auto result = ConsumeColorInterpolationMethod(tokens, state);
      if (!tokens.IsAtEnd())
      {
        return null;
      }
      return result;
    };

    auto hsl = parse(u8"in hsl");
    REQUIRE(hsl);
    REQUIRE(std::holds_alternative<ColorInterpolationMethod::HSL>(hsl->ColorSpace));
    CHECK(std::get<ColorInterpolationMethod::HSL>(hsl->ColorSpace).HueInterpolationMethod
          == HueInterpolationMethod::Shorter);
    CHECK(hsl->AlphaPremultiplication == AlphaPremultiplication::Premultiplied);

    auto longer = parse(u8"in oklch longer hue");
    REQUIRE(longer);
    CHECK(std::get<ColorInterpolationMethod::OKLCH>(longer->ColorSpace).HueInterpolationMethod
          == HueInterpolationMethod::Longer);
    CHECK(std::get<ColorInterpolationMethod::LCH>(parse(u8"in lch decreasing hue")->ColorSpace)
            .HueInterpolationMethod
          == HueInterpolationMethod::Decreasing);
    CHECK(std::get<ColorInterpolationMethod::HWB>(parse(u8"in hwb increasing hue")->ColorSpace)
            .HueInterpolationMethod
          == HueInterpolationMethod::Increasing);

    CHECK(std::holds_alternative<ColorInterpolationMethod::XYZD65>(parse(u8"in xyz")->ColorSpace));
    CHECK(std::holds_alternative<ColorInterpolationMethod::XYZD65>(parse(u8"in xyz-d65")->ColorSpace));
    CHECK(std::holds_alternative<ColorInterpolationMethod::XYZD50>(parse(u8"in xyz-d50")->ColorSpace));
    CHECK(
      std::holds_alternative<ColorInterpolationMethod::SRGBLinear>(parse(u8"in srgb-linear")->ColorSpace));
    CHECK(std::holds_alternative<ColorInterpolationMethod::DisplayP3Linear>(
      parse(u8"in display-p3-linear")->ColorSpace));
    CHECK(std::holds_alternative<ColorInterpolationMethod::OKLab>(parse(u8"in oklab")->ColorSpace));

    // Rectangular spaces don't take a hue method; polar ones need `hue` after it.
    CHECK_FALSE(parse(u8"in srgb shorter hue"));
    CHECK_FALSE(parse(u8"in hsl shorter"));
    CHECK_FALSE(parse(u8"in hsl hue"));
    CHECK_FALSE(parse(u8"in foo"));
  }

  TEST_CASE("Colour parsing(<'dynamic-range-limit'>)", "[CSS][Properties][Consumers][Color]")
  {
    ParserContext context {ParserMode::HTMLStandard};
    CHECK(ParseDynamicRangeLimit(u8"standard", context) == DynamicRangeLimit {keywords::Standard {}});
    CHECK(ParseDynamicRangeLimit(u8"no-limit", context) == DynamicRangeLimit {keywords::NoLimit {}});
    CHECK_FALSE(ParseDynamicRangeLimit(u8"constrained", context));
    CHECK_FALSE(ParseDynamicRangeLimit(u8"dynamic-range-limit-mix(standard 50%, no-limit 50%)", context));
    CHECK_FALSE(ParseDynamicRangeLimit(u8"high", context));

    ParserContext enabled {ParserMode::HTMLStandard};
    enabled.cssDynamicRangeLimitMixEnabled = true;
    enabled.cssConstrainedDynamicRangeLimitEnabled = true;
    CHECK(ParseDynamicRangeLimit(u8"constrained", enabled) == DynamicRangeLimit {keywords::Constrained {}});

    auto mix = ParseDynamicRangeLimit(u8"dynamic-range-limit-mix(standard 25%, 75% no-limit)", enabled);
    REQUIRE(mix);
    CHECK(SerialiseForTest(*mix) == "dynamic-range-limit-mix(standard 25%, no-limit 75%)");

    CHECK(ParseDynamicRangeLimit(
      u8"dynamic-range-limit-mix(dynamic-range-limit-mix(standard 50%, constrained 50%) 10%, no-limit 90%)",
      enabled));

    // At least two components, each with a percentage.
    CHECK_FALSE(ParseDynamicRangeLimit(u8"dynamic-range-limit-mix(standard 50%)", enabled));
    CHECK_FALSE(ParseDynamicRangeLimit(u8"dynamic-range-limit-mix(standard, no-limit 50%)", enabled));
    CHECK_FALSE(ParseDynamicRangeLimit(u8"dynamic-range-limit-mix(standard 101%, no-limit 50%)", enabled));
  }

  namespace
  {
    /// @brief Tokenizes `css` and consumes it with `consume`; null unless every token was consumed.
    template <typename TConsume>
    RefPtr<Value> ConsumeValue(utf8_stringview css, const ParserContext &context, TConsume &&consume)
    {
      InputStream input {CSSOMString(css)};
      Tokenizer tokenizer {input};
      tokenizer.PumpTokenizer();
      auto tokens = tokenizer.Tokens();

      PropertyParserState state {.Context = context};
      auto result = consume(tokens, state);
      if (!tokens.IsAtEnd())
      {
        return nullptr;
      }
      return result;
    }

    RefPtr<Value> ConsumeColorValue(utf8_stringview css, const ParserContext &context)
    {
      return ConsumeValue(css, context, [](TokenRange &tokens, PropertyParserState &state)
                          { return ConsumeColor(tokens, state); });
    }
  }

  TEST_CASE("Colour parsing(Value entry point)", "[CSS][Properties][Consumers][Color]")
  {
    ParserContext context {ParserMode::HTMLStandard};

    SECTION("Keywords are primitive values")
    {
      auto value = ConsumeColorValue(u8"red", context);
      REQUIRE(value);
      CHECK(value->IsPrimitiveValue());
      CHECK(value->ValueId() == ValueId::Red);
      CHECK(CssText(*value) == "red");
    }

    SECTION("Hex colours are pooled colour values")
    {
      auto value = ConsumeColorValue(u8"#ff8000", context);
      REQUIRE(value);
      CHECK(value->IsColorValue());
      CHECK(CssText(*value) == "rgb(255, 128, 0)");
      CHECK(ConsumeColorValue(u8"#ff8000", context).get() == value.get());
    }

    SECTION("Absolute colours are pooled colour values")
    {
      auto value = ConsumeColorValue(u8"rgb(0 128 255)", context);
      REQUIRE(value);
      CHECK(value->IsColorValue());
      CHECK(CssText(*value) == "rgb(0, 128, 255)");
      CHECK(ConsumeColorValue(u8"rgb(0 128 255)", context).get() == value.get());
    }

    SECTION("Unresolved colours are new colour values")
    {
      auto value = ConsumeColorValue(u8"light-dark(red, blue)", context);
      REQUIRE(value);
      CHECK(value->IsColorValue());
      CHECK(CssText(*value) == "light-dark(red, blue)");
      CHECK(ConsumeColorValue(u8"light-dark(red, blue)", context).get() != value.get());
    }

    SECTION("Invalid colours")
    {
      CHECK_FALSE(ConsumeColorValue(u8"not-a-colour", context));
      CHECK_FALSE(ConsumeColorValue(u8"#ff", context));
    }
  }

  TEST_CASE("Colour parsing(<'dynamic-range-limit'> Value)", "[CSS][Properties][Consumers][Color]")
  {
    ParserContext context {ParserMode::HTMLStandard};
    auto consume = [](TokenRange &tokens, PropertyParserState &state)
    {
      return ConsumeDynamicRangeLimit(tokens, state);
    };

    auto value = ConsumeValue(u8"no-limit", context, consume);
    REQUIRE(value);
    CHECK(value->IsDynamicRangeLimitValue());
    CHECK(CssText(*value) == "no-limit");

    CHECK_FALSE(ConsumeValue(u8"high", context, consume));
  }
}

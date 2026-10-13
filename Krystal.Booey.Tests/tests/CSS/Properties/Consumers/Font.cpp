#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/FontFaceSrcValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;
  using css::tests::Tokens;

  namespace
  {
    ParserContext Context()
    {
      return ParserContext {ParserMode::HTMLStandard, url::URL(u8"https://example.com/dir/")};
    }

    Maybe<UnresolvedFont> ConsumeFont(utf8_stringview css)
    {
      ParserContext context {ParserMode::HTMLStandard};
      PropertyParserState state {.Context = context};
      Tokens tokens {css};
      auto range = tokens.Tokenizer.Tokens();
      auto font = ConsumeUnresolvedFont(range, state);
      // The consumer only succeeds when it reaches the end.
      CHECK((!font || range.IsAtEnd()));
      return font;
    }

    Maybe<CSSOMString> ConsumeFormat(utf8_stringview css, bool rejectStringValues = false)
    {
      ParserContext context {ParserMode::HTMLStandard};
      PropertyParserState state {.Context = context};
      Tokens tokens {css};
      auto range = tokens.Tokenizer.Tokens();
      return ConsumeFontFormat(range, state, rejectStringValues);
    }

    List<FontTechnology> ConsumeTech(utf8_stringview css, bool singleValue = false)
    {
      ParserContext context {ParserMode::HTMLStandard};
      PropertyParserState state {.Context = context};
      Tokens tokens {css};
      auto range = tokens.Tokenizer.Tokens();
      return ConsumeFontTech(range, state, singleValue);
    }

    List<CSSOMStringAtom> ConsumePreludeNames(utf8_stringview css)
    {
      ParserContext context {ParserMode::HTMLStandard};
      Tokens tokens {css};
      auto range = tokens.Tokenizer.Tokens();
      return ConsumeFontFeatureValuesPreludeFamilyNameList(range, context);
    }

    constexpr auto FontFaceSrc = [](TokenRange &tokens, PropertyParserState &state) -> RefPtr<Value>
    {
      return ConsumeFontFaceSrc(tokens, state);
    };
  }

  TEST_CASE("PropertyParserHelpers(ConsumeFontStyle)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"normal", ConsumeFontStyle) == "normal");
    CHECK(ConsumeToText(u8"italic", ConsumeFontStyle) == "italic");
    CHECK(ConsumeToText(u8"oblique", ConsumeFontStyle) == "oblique");
    CHECK(ConsumeToText(u8"oblique 10deg", ConsumeFontStyle) == "oblique 10deg");
    CHECK(ConsumeToText(u8"oblique -90deg", ConsumeFontStyle) == "oblique -90deg");

    // The angle is limited to [-90deg, 90deg], and only follows `oblique`.
    CHECK(ConsumeToText(u8"oblique 91deg", ConsumeFontStyle) == "<partial>");
    CHECK(ConsumeToText(u8"italic 10deg", ConsumeFontStyle) == "<partial>");
    CHECK(ConsumeToText(u8"bold", ConsumeFontStyle) == "<null>");
    CHECK(ConsumeToText(u8"10deg", ConsumeFontStyle) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(GenericFontFamily)", "[CSS][Properties][Consumers]")
  {
    CHECK(GenericFontFamily(ValueId::Serif) == WebKitFontFamilyNames::SerifFamily());
    CHECK(GenericFontFamily(ValueId::SansSerif).View() == u8"-webkit-sans-serif");
    CHECK(GenericFontFamily(ValueId::Math).View() == u8"math");
    CHECK(GenericFontFamily(ValueId::WebkitBody) == CSSOMStringAtom::Null());
    CHECK(GenericFontFamily(ValueId::Bold) == CSSOMStringAtom::Null());

    CHECK(GenericFontFamilyIndex(ValueId::Monospace)
          == WebKitFontFamilyNames::FamilyNamesIndex::MonospaceFamily);
    CHECK(GenericFontFamilyIndex(ValueId::SystemUi)
          == WebKitFontFamilyNames::FamilyNamesIndex::SystemUiFamily);
    CHECK(WebKitFontFamilyNames::FamilyName(WebKitFontFamilyNames::FamilyNamesIndex::StandardFamily).View()
          == u8"-webkit-standard");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeFamilyName)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"Arial", ConsumeFamilyName) == "Arial");
    CHECK(ConsumeToText(u8"\"Times New Roman\"", ConsumeFamilyName) == "\"Times New Roman\"");
    CHECK(ConsumeToText(u8"Times   New Roman", ConsumeFamilyName) == "\"Times New Roman\"");

    // An unquoted generic family or CSS-wide keyword isn't a <family-name> on its own...
    CHECK(ConsumeToText(u8"serif", ConsumeFamilyName) == "<null>");
    CHECK(ConsumeToText(u8"inherit", ConsumeFamilyName) == "<null>");
    // ...but can be part of a longer one, or quoted.
    CHECK(ConsumeToText(u8"serif x", ConsumeFamilyName) == "\"serif x\"");
    // A family name is only quoted when it isn't an identifier, so a quoted generic family loses its quotes.
    CHECK(ConsumeToText(u8"\"serif\"", ConsumeFamilyName) == "serif");

    CHECK(ConsumeToText(u8"10px", ConsumeFamilyName) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeFontFamily)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"serif", ConsumeFontFamily) == "serif");
    CHECK(ConsumeToText(u8"Arial, \"Times New Roman\", sans-serif", ConsumeFontFamily)
          == "Arial, \"Times New Roman\", sans-serif");
    CHECK(ConsumeToText(u8"-webkit-body, -webkit-pictograph, math", ConsumeFontFamily)
          == "-webkit-body, -webkit-pictograph, math");
    CHECK(ConsumeToText(u8"system-ui", ConsumeFontFamily) == "system-ui");

    CHECK(ConsumeToText(u8"Arial,", ConsumeFontFamily) == "<null>");
    CHECK(ConsumeToText(u8"Arial, 10px", ConsumeFontFamily) == "<null>");
    CHECK(ConsumeToText(u8"initial", ConsumeFontFamily) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeFontSizeAdjust)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeFontSizeAdjust) == "none");
    CHECK(ConsumeToText(u8"from-font", ConsumeFontSizeAdjust) == "from-font");
    CHECK(ConsumeToText(u8"0.5", ConsumeFontSizeAdjust) == "0.5");
    CHECK(ConsumeToText(u8"ex-height 0.5", ConsumeFontSizeAdjust) == "0.5");
    CHECK(ConsumeToText(u8"cap-height 0.5", ConsumeFontSizeAdjust) == "cap-height 0.5");
    CHECK(ConsumeToText(u8"ic-width from-font", ConsumeFontSizeAdjust) == "ic-width from-font");

    CHECK(ConsumeToText(u8"-1", ConsumeFontSizeAdjust) == "<null>");
    CHECK(ConsumeToText(u8"cap-height", ConsumeFontSizeAdjust) == "<null>");
    CHECK(ConsumeToText(u8"0.5 cap-height", ConsumeFontSizeAdjust) == "<partial>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeUnresolvedFont)", "[CSS][Properties][Consumers]")
  {
    SECTION("Size and family only")
    {
      auto font = ConsumeFont(u8"12px serif");
      REQUIRE(font);
      CHECK(font->Style == UnresolvedFontStyle {ValueId::Normal});
      CHECK(font->VariantCaps == ValueId::Normal);
      CHECK(font->Weight == UnresolvedFontWeight {ValueId::Normal});
      CHECK(font->Width == UnresolvedFontWidth {ValueId::Normal});
      CHECK(std::holds_alternative<LengthPercentage<NonNegative>>(font->Size));
      CHECK(font->LineHeight == UnresolvedFontLineHeight {ValueId::Normal});
      REQUIRE(font->Family.size() == 1);
      CHECK(font->Family[0] == UnresolvedFontFamilyName {ValueId::Serif});
    }

    SECTION("Every component")
    {
      auto font =
        ConsumeFont(u8"italic small-caps bold condensed larger/1.5 \"Helvetica Neue\", Times New Roman, "
                    u8"sans-serif");
      REQUIRE(font);
      CHECK(font->Style == UnresolvedFontStyle {ValueId::Italic});
      CHECK(font->VariantCaps == ValueId::SmallCaps);
      CHECK(font->Weight == UnresolvedFontWeight {ValueId::Bold});
      CHECK(font->Width == UnresolvedFontWidth {ValueId::Condensed});
      CHECK(font->Size == UnresolvedFontSize {ValueId::Larger});
      CHECK(std::holds_alternative<Number<NonNegative>>(font->LineHeight));
      REQUIRE(font->Family.size() == 3);
      CHECK(font->Family[0] == UnresolvedFontFamilyName {CSSOMStringAtom {u8"Helvetica Neue"}});
      CHECK(font->Family[1] == UnresolvedFontFamilyName {CSSOMStringAtom {u8"Times New Roman"}});
      CHECK(font->Family[2] == UnresolvedFontFamilyName {ValueId::SansSerif});
    }

    SECTION("Optional components in any order, with `normal` placeholders")
    {
      auto font = ConsumeFont(u8"500 normal oblique 20deg 10% a");
      REQUIRE(font);
      CHECK(std::holds_alternative<UnresolvedFontStyleObliqueAngle>(font->Style));
      CHECK(std::holds_alternative<UnresolvedFontWeightNumber>(font->Weight));
      CHECK(std::holds_alternative<LengthPercentage<NonNegative>>(font->Size));

      CHECK(ConsumeFont(u8"normal normal normal normal 12px a"));
      CHECK(ConsumeFont(u8"bold 12px/normal a"));
      CHECK(ConsumeFont(u8"12px/20px a"));
    }

    SECTION("Font size keywords")
    {
      auto font = ConsumeFont(u8"-webkit-xxx-large a");
      REQUIRE(font);
      CHECK(font->Size == UnresolvedFontSize {ValueId::XxxLarge});

      font = ConsumeFont(u8"math a");
      REQUIRE(font);
      CHECK(font->Size == UnresolvedFontSize {ValueId::Math});
    }

    SECTION("Invalid")
    {
      CHECK_FALSE(ConsumeFont(u8"bold"));
      CHECK_FALSE(ConsumeFont(u8"12px"));
      CHECK_FALSE(ConsumeFont(u8"12px/ a"));
      CHECK_FALSE(ConsumeFont(u8"12px a,"));
      CHECK_FALSE(ConsumeFont(u8"-1px a"));
      CHECK_FALSE(ConsumeFont(u8"bold bold 12px a"));
      CHECK_FALSE(ConsumeFont(u8"italic bold small-caps condensed normal 12px a"));
      // The font weight number is limited to [1, 1000].
      CHECK_FALSE(ConsumeFont(u8"1001 12px a"));
    }
  }

  TEST_CASE("PropertyParserHelpers(ConsumeFontFormat)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeFormat(u8"format(woff2)") == CSSOMString {u8"woff2"});
    CHECK(ConsumeFormat(u8"format(embedded-opentype)") == CSSOMString {u8"embedded-opentype"});
    CHECK(ConsumeFormat(u8"format(\"truetype-aat\")") == CSSOMString {u8"truetype-aat"});
    // An empty string is still a format.
    CHECK(ConsumeFormat(u8"format(\"\")") == CSSOMString {});

    CHECK_FALSE(ConsumeFormat(u8"format(\"woff\")", true));
    CHECK_FALSE(ConsumeFormat(u8"format(foo)"));
    CHECK_FALSE(ConsumeFormat(u8"format(woff, woff2)"));
    CHECK_FALSE(ConsumeFormat(u8"format()"));
  }

  TEST_CASE("PropertyParserHelpers(ConsumeFontTech)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeTech(u8"tech(variations)") == List<FontTechnology> {FontTechnology::Variations});
    CHECK(ConsumeTech(u8"tech(color-COLRv1, features-opentype, palettes)")
          == List<FontTechnology> {FontTechnology::ColorColrv1, FontTechnology::FeaturesOpentype,
                                   FontTechnology::Palettes});

    CHECK(ConsumeTech(u8"tech(variations, palettes)", true).empty());
    CHECK(ConsumeTech(u8"tech(foo)").empty());
    CHECK(ConsumeTech(u8"tech(\"variations\")").empty());
    CHECK(ConsumeTech(u8"tech(variations,)").empty());
  }

  TEST_CASE("PropertyParserHelpers(ConsumeFontFaceSrc)", "[CSS][Properties][Consumers]")
  {
    auto context = Context();

    CHECK(ConsumeToText(u8"url(a.woff2)", FontFaceSrc, context) == "url(\"a.woff2\")");
    CHECK(ConsumeToText(u8"url(a.woff2) format(woff2)", FontFaceSrc, context)
          == "url(\"a.woff2\") format(\"woff2\")");
    CHECK(ConsumeToText(u8"url(a.otf) format(\"opentype\") tech(variations, color-svg)", FontFaceSrc, context)
          == "url(\"a.otf\") format(\"opentype\") tech(variations, color-svg)");
    CHECK(ConsumeToText(u8"local(Foo  Bar), local(\"Baz\")", FontFaceSrc, context)
          == "local(\"Foo Bar\"), local(\"Baz\")");

    // Invalid components are dropped from the list.
    CHECK(ConsumeToText(u8"foo, url(a.woff) format(foo), local(serif), url(b.woff)", FontFaceSrc, context)
          == "url(\"b.woff\")");
    CHECK(ConsumeToText(u8"url(a.woff) tech(), url(b.woff) bar", FontFaceSrc, context) == "<null>");
    CHECK(ConsumeToText(u8"local(a, b)", FontFaceSrc, context) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeFontFaceFontStyle)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"normal", ConsumeFontFaceFontStyle) == "normal");
    CHECK(ConsumeToText(u8"italic", ConsumeFontFaceFontStyle) == "italic");
    CHECK(ConsumeToText(u8"oblique", ConsumeFontFaceFontStyle) == "oblique");
    CHECK(ConsumeToText(u8"oblique 10deg", ConsumeFontFaceFontStyle) == "oblique 10deg");
    CHECK(ConsumeToText(u8"oblique 10deg 20deg", ConsumeFontFaceFontStyle) == "oblique 10deg 20deg");

    CHECK(ConsumeToText(u8"oblique 100deg", ConsumeFontFaceFontStyle) == "<null>");
    CHECK(ConsumeToText(u8"oblique 10deg 100deg", ConsumeFontFaceFontStyle) == "<null>");
    CHECK(ConsumeToText(u8"oblique 10deg 20deg 30deg", ConsumeFontFaceFontStyle) == "<partial>");
    CHECK(ConsumeToText(u8"auto", ConsumeFontFaceFontStyle) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeFeatureTagValue)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"\"liga\"", ConsumeFeatureTagValue) == "\"liga\"");
    CHECK(ConsumeToText(u8"\"liga\" on", ConsumeFeatureTagValue) == "\"liga\"");
    CHECK(ConsumeToText(u8"\"liga\" off", ConsumeFeatureTagValue) == "\"liga\" 0");
    CHECK(ConsumeToText(u8"\"liga\" 0", ConsumeFeatureTagValue) == "\"liga\" 0");
    CHECK(ConsumeToText(u8"\"swsh\" 2", ConsumeFeatureTagValue) == "\"swsh\" 2");
    CHECK(ConsumeToText(u8"\"a b \"", ConsumeFeatureTagValue) == "\"a b \"");

    CHECK(ConsumeToText(u8"\"lig\"", ConsumeFeatureTagValue) == "<null>");
    CHECK(ConsumeToText(u8"\"ligat\"", ConsumeFeatureTagValue) == "<null>");
    CHECK(ConsumeToText(u8"\"ligé\"", ConsumeFeatureTagValue) == "<null>");
    CHECK(ConsumeToText(u8"liga", ConsumeFeatureTagValue) == "<null>");
    CHECK(ConsumeToText(u8"\"liga\" -1", ConsumeFeatureTagValue) == "<null>");
    CHECK(ConsumeToText(u8"\"liga\" 1.5", ConsumeFeatureTagValue) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeVariationTagValue)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"\"wght\" 400", ConsumeVariationTagValue) == "\"wght\" 400");
    CHECK(ConsumeToText(u8"\"wdth\" -1.5", ConsumeVariationTagValue) == "\"wdth\" -1.5");

    CHECK(ConsumeToText(u8"\"wght\"", ConsumeVariationTagValue) == "<null>");
    CHECK(ConsumeToText(u8"\"wght\" on", ConsumeVariationTagValue) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeFontFeatureValuesPreludeFamilyNameList)",
            "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumePreludeNames(u8"Foo, \"Bar Baz\", Qux  Quux")
          == List<CSSOMStringAtom> {CSSOMStringAtom {u8"Foo"}, CSSOMStringAtom {u8"Bar Baz"},
                                    CSSOMStringAtom {u8"Qux Quux"}});

    CHECK(ConsumePreludeNames(u8"serif").empty());
    CHECK(ConsumePreludeNames(u8"Foo, serif").empty());
    CHECK(ConsumePreludeNames(u8"Foo,").empty());
  }

  TEST_CASE("PropertyParserHelpers(IsSystemFontShorthand)", "[CSS][Properties][Consumers]")
  {
    CHECK(IsSystemFontShorthand(ValueId::Caption));
    CHECK(IsSystemFontShorthand(ValueId::StatusBar));
    CHECK_FALSE(IsSystemFontShorthand(ValueId::Serif));
  }
}

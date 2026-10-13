#include "Krystal.Booey/CSS/Values/FontFaceSrcValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("FontFaceSrcValue(CssTextFromFontTech)", "[CSS][Values][FontFaceSrcValue]")
  {
    STATIC_REQUIRE(CssTextFromFontTech(FontTechnology::ColorColrv0) == u8"color-colrv0");
    STATIC_REQUIRE(CssTextFromFontTech(FontTechnology::ColorColrv1) == u8"color-colrv1");
    STATIC_REQUIRE(CssTextFromFontTech(FontTechnology::ColorCbdt) == u8"color-cbdt");
    STATIC_REQUIRE(CssTextFromFontTech(FontTechnology::ColorSbix) == u8"color-sbix");
    STATIC_REQUIRE(CssTextFromFontTech(FontTechnology::ColorSvg) == u8"color-svg");
    STATIC_REQUIRE(CssTextFromFontTech(FontTechnology::FeaturesAat) == u8"features-aat");
    STATIC_REQUIRE(CssTextFromFontTech(FontTechnology::FeaturesGraphite) == u8"features-graphite");
    STATIC_REQUIRE(CssTextFromFontTech(FontTechnology::FeaturesOpentype) == u8"features-opentype");
    STATIC_REQUIRE(CssTextFromFontTech(FontTechnology::Incremental) == u8"incremental");
    STATIC_REQUIRE(CssTextFromFontTech(FontTechnology::Palettes) == u8"palettes");
    STATIC_REQUIRE(CssTextFromFontTech(FontTechnology::Variations) == u8"variations");
    STATIC_REQUIRE(CssTextFromFontTech(FontTechnology::Invalid).empty());
  }
}

namespace krys::boo::css::tests
{
  namespace
  {
    CSSURL FontURL(utf8_stringview url)
    {
      return *CompleteURL(CSSOMString {url},
                          ParserContext {ParserMode::HTMLStandard, url::URL {u8"https://example.com/"}});
    }
  }

  TEST_CASE_METHOD(ValueFixture, "FontFaceSrcLocalValue::Create", "[CSS][Values][FontFaceSrcLocalValue]")
  {
    auto value = FontFaceSrcLocalValue::Create(CSSOMStringAtom {u8"Helvetica Neue"});
    CHECK(value->IsFontFaceSrcLocalValue());
    CHECK_FALSE(value->IsFontFaceSrcResourceValue());
    CHECK_FALSE(value->IsImage());
    CHECK_FALSE(value->IsEmpty());
    CHECK(value->FontFaceName() == CSSOMStringAtom {u8"Helvetica Neue"});

    CHECK(FontFaceSrcLocalValue::Create(CSSOMStringAtom {u8""})->IsEmpty());
  }

  TEST_CASE_METHOD(ValueFixture, "FontFaceSrcLocalValue serialisation",
                   "[CSS][Values][FontFaceSrcLocalValue]")
  {
    // The name is serialised as a string.
    CHECK(CssText(*FontFaceSrcLocalValue::Create(CSSOMStringAtom {u8"Helvetica Neue"}))
          == "local(\"Helvetica Neue\")");
    CHECK(CssText(*FontFaceSrcLocalValue::Create(CSSOMStringAtom {u8"a\"b"})) == "local(\"a\\\"b\")");
  }

  TEST_CASE_METHOD(ValueFixture, "FontFaceSrcLocalValue equality and children",
                   "[CSS][Values][FontFaceSrcLocalValue]")
  {
    auto a = FontFaceSrcLocalValue::Create(CSSOMStringAtom {u8"Arial"});
    auto b = FontFaceSrcLocalValue::Create(CSSOMStringAtom {u8"Arial"});
    auto c = FontFaceSrcLocalValue::Create(CSSOMStringAtom {u8"Times"});
    CHECK(*a == *b);
    CHECK_FALSE(*a == *c);
    CHECK_FALSE(*a == *FontFaceSrcResourceValue::Create(FontURL(u8"Arial"), {}, {}));

    // Not hashable: only values that define a derived hash are.
    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));

    CHECK(Children(*a).empty());
    CHECK_FALSE(a->MayDependOnBaseUrl());
    CHECK(a->ComputedStyleDependencies().IsComputationallyIndependent());
  }

  TEST_CASE_METHOD(ValueFixture, "FontFaceSrcResourceValue::Create",
                   "[CSS][Values][FontFaceSrcResourceValue]")
  {
    auto value =
      FontFaceSrcResourceValue::Create(FontURL(u8"a.woff2"), u8"woff2", {FontTechnology::Variations});
    CHECK(value->IsFontFaceSrcResourceValue());
    CHECK_FALSE(value->IsFontFaceSrcLocalValue());
    CHECK_FALSE(value->IsEmpty());

    CHECK(FontFaceSrcResourceValue::Create(CSSURL::None(), {}, {})->IsEmpty());
  }

  TEST_CASE_METHOD(ValueFixture, "FontFaceSrcResourceValue serialisation",
                   "[CSS][Values][FontFaceSrcResourceValue]")
  {
    CHECK(CssText(*FontFaceSrcResourceValue::Create(FontURL(u8"a.woff"), {}, {})) == "url(\"a.woff\")");
    CHECK(CssText(*FontFaceSrcResourceValue::Create(FontURL(u8"a.woff2"), u8"woff2", {}))
          == "url(\"a.woff2\") format(\"woff2\")");
    CHECK(CssText(*FontFaceSrcResourceValue::Create(FontURL(u8"a.ttf"), {}, {FontTechnology::Variations}))
          == "url(\"a.ttf\") tech(variations)");
    CHECK(CssText(*FontFaceSrcResourceValue::Create(FontURL(u8"a.ttf"), u8"opentype",
                                                    {FontTechnology::ColorColrv1, FontTechnology::Palettes}))
          == "url(\"a.ttf\") format(\"opentype\") tech(color-colrv1, palettes)");
  }

  TEST_CASE_METHOD(ValueFixture, "FontFaceSrcResourceValue equality",
                   "[CSS][Values][FontFaceSrcResourceValue]")
  {
    auto make = [](utf8_stringview url, CSSOMString format, List<FontTechnology> technologies)
    {
      return FontFaceSrcResourceValue::Create(FontURL(url), krys::move(format), krys::move(technologies));
    };

    auto a = make(u8"a.woff2", u8"woff2", {FontTechnology::Variations});
    CHECK(*a == *make(u8"a.woff2", u8"woff2", {FontTechnology::Variations}));
    CHECK_FALSE(*a == *make(u8"b.woff2", u8"woff2", {FontTechnology::Variations}));
    CHECK_FALSE(*a == *make(u8"a.woff2", u8"woff", {FontTechnology::Variations}));
    CHECK_FALSE(*a == *make(u8"a.woff2", u8"woff2", {}));
    CHECK_FALSE(*a == *make(u8"a.woff2", u8"woff2", {FontTechnology::Variations, FontTechnology::Palettes}));
  }

  TEST_CASE_METHOD(ValueFixture, "FontFaceSrcResourceValue resolves its URL",
                   "[CSS][Values][FontFaceSrcResourceValue]")
  {
    // A URL parsed without a base is resolved on creation, so absolute URLs compare equal to resolved ones.
    auto unresolved = *CompleteURL(u8"https://example.com/a.woff", ParserContext {ParserMode::HTMLStandard});
    REQUIRE(unresolved.Resolved.IsNull());
    auto value = FontFaceSrcResourceValue::Create(krys::move(unresolved), {}, {});
    CHECK(*value == *FontFaceSrcResourceValue::Create(FontURL(u8"https://example.com/a.woff"), {}, {}));
  }

  TEST_CASE_METHOD(ValueFixture, "FontFaceSrcResourceValue base URL, subresources and children",
                   "[CSS][Values][FontFaceSrcResourceValue]")
  {
    CHECK(FontFaceSrcResourceValue::Create(FontURL(u8"a.woff"), {}, {})->MayDependOnBaseUrl());
    CHECK_FALSE(FontFaceSrcResourceValue::Create(FontURL(u8"#font"), {}, {})->MayDependOnBaseUrl());
    CHECK_FALSE(FontFaceSrcResourceValue::Create(FontURL(u8"data:font/woff2;base64,AAAA"), {}, {})
                  ->MayDependOnBaseUrl());

    // No font is loaded, so there are no subresources.
    auto value = FontFaceSrcResourceValue::Create(FontURL(u8"a.woff"), {}, {});
    bool called = false;
    CHECK_FALSE(value->TraverseSubresources(
      [&](const CachedResource &)
      {
        called = true;
        return true;
      }));
    CHECK_FALSE(called);

    CHECK(Children(*value).empty());
  }
}

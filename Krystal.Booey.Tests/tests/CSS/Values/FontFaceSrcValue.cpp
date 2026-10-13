#include "Krystal.Booey/CSS/Values/FontFaceSrcValue.hpp"
#include "catch2/catch_test_macros.hpp"

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

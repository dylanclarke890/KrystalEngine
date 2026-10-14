#include "Krystal.Booey/CSS/CSSOM/CSSFontPaletteValuesRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSFontPaletteValuesRule", "[CSS][CSSOM][CSSFontPaletteValuesRule]")
  {
    auto sheet = SheetFromText(u8"@font-palette-values --one { font-family: Font One, b; base-palette: 2; "
                               u8"override-colors: 0 red, 3 rgb(0, 0, 255); } "
                               u8"@font-palette-values --two { base-palette: dark; } "
                               u8"@font-palette-values --three { base-palette: light; } "
                               u8"@font-palette-values --four { }");

    SECTION("everything")
    {
      auto &rule = RuleAt<CSSFontPaletteValuesRule>(*sheet, 0);
      CHECK(TypeOf(rule) == RuleType::FontPaletteValues);
      CHECK(ToNarrow(rule.Name()) == "--one");
      CHECK(ToNarrow(rule.FontFamily()) == "\"Font One\", b");
      CHECK(ToNarrow(rule.BasePalette()) == "2");
      CHECK(ToNarrow(rule.OverrideColors()) == "0 rgb(255, 0, 0), 3 rgb(0, 0, 255)");
      CHECK(RuleText(rule)
            == "@font-palette-values --one { font-family: \"Font One\", b; base-palette: 2; "
               "override-colors: 0 rgb(255, 0, 0), 3 rgb(0, 0, 255); }");
    }

    SECTION("keyword base palettes")
    {
      auto &dark = RuleAt<CSSFontPaletteValuesRule>(*sheet, 1);
      CHECK(ToNarrow(dark.BasePalette()) == "dark");
      CHECK(RuleText(dark) == "@font-palette-values --two { base-palette: dark; }");

      auto &light = RuleAt<CSSFontPaletteValuesRule>(*sheet, 2);
      CHECK(ToNarrow(light.BasePalette()) == "light");
      CHECK(RuleText(light) == "@font-palette-values --three { base-palette: light; }");
    }

    SECTION("nothing")
    {
      auto &rule = RuleAt<CSSFontPaletteValuesRule>(*sheet, 3);
      CHECK(rule.FontFamily().empty());
      CHECK(rule.BasePalette().empty());
      CHECK(rule.OverrideColors().empty());
      CHECK(RuleText(rule) == "@font-palette-values --four { }");
    }
  }
}

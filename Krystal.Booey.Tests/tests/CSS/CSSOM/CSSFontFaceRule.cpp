#include "Krystal.Booey/CSS/CSSOM/CSSFontFaceRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey/CSS/Descriptors/CSSFontFaceDescriptors.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSFontFaceRule", "[CSS][CSSOM][CSSFontFaceRule]")
  {
    auto sheet = SheetFromText(u8"@font-face { } @font-face { font-family: a; src: url(a.woff); }");

    CHECK(sheet->Item(0)->StyleRuleType() == RuleType::FontFace);
    CHECK(RuleText(*sheet->Item(0)) == "@font-face { }");
    CHECK(RuleText(*sheet->Item(1)) == "@font-face { font-family: a; src: url(\"a.woff\"); }");
    CHECK(ToNarrow(sheet->Item(1)->CssText(DefaultSerialisationContext()))
          == "@font-face { font-family: a; src: url(\"a.woff\"); }");

    SECTION("style")
    {
      auto &rule = RuleAt<CSSFontFaceRule>(*sheet, 1);
      auto &style = rule.Style();
      CHECK(&style == &rule.Style());
      CHECK(AsDeclaration(style).ParentRule() == &rule);
      CHECK(ToNarrow(style.FontFamily()) == "a");

      REQUIRE_FALSE(style.SetFontDisplay(u8"swap").HasException());
      CHECK(RuleText(rule) == "@font-face { font-family: a; src: url(\"a.woff\"); font-display: swap; }");
    }
  }
}

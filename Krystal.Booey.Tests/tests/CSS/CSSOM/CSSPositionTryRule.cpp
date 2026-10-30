#include "Krystal.Booey/CSS/CSSOM/CSSPositionTryRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey/CSS/Descriptors/CSSPositionTryDescriptors.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSPositionTryRule", "[CSS][CSSOM][CSSPositionTryRule]")
  {
    auto sheet = SheetFromText(u8"@position-try --a { top: 1px; margin: 2px; } @position-try --b { }");
    auto &rule = RuleAt<CSSPositionTryRule>(*sheet, 0);

    CHECK(rule.StyleRuleType() == RuleType::PositionTry);
    CHECK(ToNarrow(CSSOMString(rule.Name().View())) == "--a");
    CHECK(RuleText(rule) == "@position-try --a { top: 1px; margin: 2px; }");
    CHECK(RuleText(*sheet->Item(1)) == "@position-try --b { }");

    SECTION("style")
    {
      auto &style = rule.Style();
      CHECK(&style == &rule.Style());
      CHECK(AsDeclaration(style).ParentRule() == &rule);
      CHECK(ToNarrow(style.Top()) == "1px");
      CHECK(ToNarrow(style.MarginBlockStart()).empty());
      CHECK(ToNarrow(style.MarginTop()) == "2px");

      REQUIRE_FALSE(style.SetPositionArea(u8"top left").HasException());
      // The horizontal keyword comes first.
      CHECK(ToNarrow(style.PositionArea()) == "left top");
      REQUIRE_FALSE(style.SetTop(u8"anchor(bottom)").HasException());
      CHECK(RuleText(rule)
            == "@position-try --a { top: anchor(bottom); margin: 2px; position-area: left top; }");

      // Properties that aren't position-try descriptors aren't exposed.
      CHECK(AsDeclaration(style).GetPropertyValue(u8"color").empty());
      REQUIRE_FALSE(AsDeclaration(style).SetProperty(u8"color", u8"red", u8"").HasException());
      CHECK(RuleText(rule)
            == "@position-try --a { top: anchor(bottom); margin: 2px; position-area: left top; }");
    }
  }
}

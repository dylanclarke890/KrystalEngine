#include "Krystal.Booey/CSS/CSSOM/CSSPageRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey/CSS/Descriptors/CSSPageDescriptors.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSPageRule", "[CSS][CSSOM][CSSPageRule]")
  {
    auto sheet = SheetFromText(u8"@page { } @page :first { margin: 1px; size: a4; }");

    SECTION("without a selector")
    {
      auto &rule = RuleAt<CSSPageRule>(*sheet, 0);
      CHECK(TypeOf(rule) == RuleType::Page);
      CHECK(rule.SelectorText().empty());
      CHECK(RuleText(rule) == "@page { }");
    }

    SECTION("with a selector and declarations")
    {
      auto &rule = RuleAt<CSSPageRule>(*sheet, 1);
      CHECK(ToNarrow(rule.SelectorText()) == ":first");
      CHECK(RuleText(rule) == "@page :first { margin: 1px; size: a4; }");
    }

    SECTION("style")
    {
      auto &rule = RuleAt<CSSPageRule>(*sheet, 1);
      auto &style = rule.Style();
      CHECK(&style == &rule.Style());
      CHECK(AsDeclaration(style).ParentRule() == &rule);
      CHECK(AsDeclaration(style).ParentStyleSheet() == sheet.get());
      CHECK(ToNarrow(style.Margin()) == "1px");
      CHECK(ToNarrow(style.MarginTop()) == "1px");

      REQUIRE_FALSE(style.SetMarginLeft(u8"2px").HasException());
      CHECK(ToNarrow(style.Margin()) == "1px 1px 1px 2px");
      CHECK(RuleText(rule) == "@page :first { margin: 1px 1px 1px 2px; size: a4; }");
    }

    SECTION("set the selector")
    {
      auto &rule = RuleAt<CSSPageRule>(*sheet, 0);
      rule.SetSelectorText(u8"a");
      CHECK(ToNarrow(rule.SelectorText()) == "a");
      CHECK(RuleText(rule) == "@page a { }");

      rule.SetSelectorText(u8"a >");
      CHECK(ToNarrow(rule.SelectorText()) == "a");
    }
  }
}

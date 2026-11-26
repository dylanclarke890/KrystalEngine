#include "Krystal.Web/Css/CSSOM/CSSNestedDeclarations.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/CSSOMTestUtils.hpp"
#include "Krystal.Web/Css/CSSOM/CSSStyleProperties.hpp"
#include "Krystal.Web/Css/CSSOM/CSSStyleRule.hpp"

namespace krys::web::css::tests
{
  TEST_CASE("CSSNestedDeclarations", "[CSS][CSSOM][CSSNestedDeclarations]")
  {
    auto sheet = SheetFromText(u8"a { color: red; & b { } color: blue; }");
    auto &outer = RuleAt<CSSStyleRule>(*sheet, 0);
    REQUIRE(outer.Length() == 2);
    REQUIRE(Is<CSSNestedDeclarations>(*outer.Item(1)));
    auto &declarations = Downcast<CSSNestedDeclarations>(*outer.Item(1));

    CHECK(TypeOf(declarations) == RuleType::NestedDeclarations);
    CHECK(declarations.ParentRule() == &outer);
    CHECK(declarations.ParentStyleSheet() == sheet.get());
    CHECK(RuleText(declarations) == "color: blue;");

    auto &style = declarations.Style();
    CHECK(&style == &declarations.Style());
    CHECK(style.ParentRule() == &declarations);
    REQUIRE_FALSE(style.SetProperty(u8"color", u8"green", u8"").HasException());
    CHECK(RuleText(declarations) == "color: green;");
    CHECK(RuleText(outer) == "a {\n  color: red;\n  & b { }\n  color: green;\n}");
  }

  TEST_CASE("CSSNestedDeclarations: its wrapper follows a copied sheet",
            "[CSS][CSSOM][CSSNestedDeclarations]")
  {
    auto sheet = SheetFromText(u8"a { & b { } color: blue; }");
    auto &rule = RuleAt<CSSStyleRule>(*sheet, 0);
    auto &declarations = Downcast<CSSNestedDeclarations>(*rule.Item(1));
    auto &style = declarations.Style();

    // Inserting a rule into the sheet can copy its contents, which reattaches the wrappers.
    REQUIRE_FALSE(sheet->InsertRule(u8"c { }", 1).HasException());
    REQUIRE_FALSE(style.SetProperty(u8"color", u8"green", u8"").HasException());
    CHECK(RuleText(declarations) == "color: green;");
    CHECK(SheetText(*sheet).find("color: green;") != std::string::npos);
  }
}

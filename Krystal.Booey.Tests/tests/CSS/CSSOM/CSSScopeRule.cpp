#include "Krystal.Booey/CSS/CSSOM/CSSScopeRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSNestedDeclarations.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSScopeRule", "[CSS][CSSOM][CSSScopeRule]")
  {
    auto sheet = SheetFromText(u8"@scope (.a, .b) to (.c) { d { } } @scope { } @scope to (.e) { }");

    SECTION("start and end")
    {
      auto &rule = RuleAt<CSSScopeRule>(*sheet, 0);
      CHECK(TypeOf(rule) == RuleType::Scope);
      CHECK(ToNarrow(rule.Start()) == ".a, .b");
      CHECK(ToNarrow(rule.End()) == ".c");
      CHECK(RuleText(rule) == "@scope (.a, .b) to (.c) {\n  d { }\n}");
    }

    SECTION("neither")
    {
      auto &rule = RuleAt<CSSScopeRule>(*sheet, 1);
      CHECK(rule.Start().empty());
      CHECK(rule.End().empty());
      CHECK(RuleText(rule) == "@scope {\n}");
    }

    SECTION("only an end")
    {
      CHECK(RuleText(*sheet->Item(2)) == "@scope to (.e) {\n}");
    }

    SECTION("declarations can be inserted")
    {
      auto &rule = RuleAt<CSSScopeRule>(*sheet, 1);
      REQUIRE(rule.InsertRule(u8"color: red", 0).HasValue());
      CHECK(Is<CSSNestedDeclarations>(*rule.Item(0)));
      CHECK(RuleText(rule) == "@scope {\n  color: red;\n}");
    }
  }
}

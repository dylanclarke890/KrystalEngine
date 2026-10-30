#include "Krystal.Booey/CSS/CSSOM/CSSPropertyRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSPropertyRule", "[CSS][CSSOM][CSSPropertyRule]")
  {
    auto sheet =
      SheetFromText(u8"@property --a { syntax: \"<length>\"; inherits: true; initial-value: 10px; } "
                    u8"@property --b { syntax: \"*\"; inherits: false; }");

    SECTION("everything")
    {
      auto &rule = RuleAt<CSSPropertyRule>(*sheet, 0);
      CHECK(TypeOf(rule) == RuleType::Property);
      CHECK(ToNarrow(rule.Name()) == "--a");
      CHECK(ToNarrow(rule.Syntax()) == "<length>");
      CHECK(rule.Inherits());
      REQUIRE(rule.InitialValue());
      CHECK(ToNarrow(*rule.InitialValue()) == "10px");
      CHECK(RuleText(rule) == "@property --a { syntax: \"<length>\"; inherits: true; initial-value: 10px; }");
    }

    SECTION("no initial value")
    {
      auto &rule = RuleAt<CSSPropertyRule>(*sheet, 1);
      CHECK_FALSE(rule.Inherits());
      CHECK_FALSE(rule.InitialValue());
      CHECK(RuleText(rule) == "@property --b { syntax: \"*\"; inherits: false; }");
    }
  }
}

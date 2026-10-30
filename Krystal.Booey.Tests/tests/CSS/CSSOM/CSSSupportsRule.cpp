#include "Krystal.Booey/CSS/CSSOM/CSSSupportsRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSSupportsRule", "[CSS][CSSOM][CSSSupportsRule]")
  {
    auto sheet = SheetFromText(u8"@supports (display: grid) and (not (color: red)) { a { color: red; } }");
    auto &rule = RuleAt<CSSSupportsRule>(*sheet, 0);

    CHECK(TypeOf(rule) == RuleType::Supports);
    CHECK(ToNarrow(rule.ConditionText()) == "(display: grid) and (not (color: red))");
    CHECK(RuleText(rule) == "@supports (display: grid) and (not (color: red)) {\n  a { color: red; }\n}");
    CHECK(ToNarrow(rule.CssText(DefaultSerialisationContext()))
          == "@supports (display: grid) and (not (color: red)) {\n  a { color: red; }\n}");
  }
}

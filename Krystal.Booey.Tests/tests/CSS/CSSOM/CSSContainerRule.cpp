#include "Krystal.Booey/CSS/CSSOM/CSSContainerRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSContainerRule", "[CSS][CSSOM][CSSContainerRule]")
  {
    auto sheet =
      SheetFromText(u8"@container card (min-width: 100px) { a { } } @container (orientation: portrait) { }");

    SECTION("named")
    {
      auto &rule = RuleAt<CSSContainerRule>(*sheet, 0);
      CHECK(TypeOf(rule) == RuleType::Container);
      CHECK(ToNarrow(rule.ContainerName()) == "card");
      CHECK(ToNarrow(rule.ContainerQuery()) == "(min-width: 100px)");
      CHECK(ToNarrow(rule.ConditionText()) == "card (min-width: 100px)");
      CHECK(RuleText(rule) == "@container card (min-width: 100px) {\n  a { }\n}");
    }

    SECTION("unnamed")
    {
      auto &rule = RuleAt<CSSContainerRule>(*sheet, 1);
      CHECK(rule.ContainerName().empty());
      CHECK(ToNarrow(rule.ContainerQuery()) == "(orientation: portrait)");
      CHECK(RuleText(rule) == "@container (orientation: portrait) {\n}");
    }
  }
}

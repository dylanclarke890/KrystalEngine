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

  TEST_CASE("CSSContainerRule: a list of conditions", "[CSS][CSSOM][CSSContainerRule]")
  {
    auto sheet = SheetFromText(u8"@container card (min-width: 100px), (orientation: portrait) { }");
    auto &rule = RuleAt<CSSContainerRule>(*sheet, 0);
    CHECK(ToNarrow(rule.ConditionText()) == "card (min-width: 100px), (orientation: portrait)");
    // Only a single condition gives its name and query.
    CHECK(rule.ContainerName().empty());
    CHECK(rule.ContainerQuery().empty());
    auto conditions = rule.Conditions();
    REQUIRE(conditions.size() == 2u);
    CHECK(ToNarrow(conditions[0].Name) == "card");
    CHECK(ToNarrow(conditions[0].Query) == "(min-width: 100px)");
    CHECK(conditions[1].Name.empty());
    CHECK(ToNarrow(conditions[1].Query) == "(orientation: portrait)");
  }
}

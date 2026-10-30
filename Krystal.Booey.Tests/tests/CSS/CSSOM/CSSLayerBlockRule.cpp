#include "Krystal.Booey/CSS/CSSOM/CSSLayerBlockRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSLayerBlockRule", "[CSS][CSSOM][CSSLayerBlockRule]")
  {
    auto sheet = SheetFromText(u8"@layer a.b { c { } } @layer { }");

    SECTION("named")
    {
      auto &rule = RuleAt<CSSLayerBlockRule>(*sheet, 0);
      CHECK(TypeOf(rule) == RuleType::LayerBlock);
      CHECK(ToNarrow(rule.Name()) == "a.b");
      CHECK(RuleText(rule) == "@layer a.b {\n  c { }\n}");
    }

    SECTION("anonymous")
    {
      auto &rule = RuleAt<CSSLayerBlockRule>(*sheet, 1);
      CHECK(rule.Name().empty());
      CHECK(RuleText(rule) == "@layer {\n}");
    }
  }

  TEST_CASE("CSSLayerBlockRule: StringFromCascadeLayerName", "[CSS][CSSOM][CSSLayerBlockRule]")
  {
    CHECK(ToNarrow(StringFromCascadeLayerName({u8"a"})) == "a");
    CHECK(ToNarrow(StringFromCascadeLayerName({u8"a", u8"b", u8"c"})) == "a.b.c");
    // Each segment is escaped as an identifier.
    CHECK(ToNarrow(StringFromCascadeLayerName({u8"1a", u8"b c"})) == "\\31 a.b\\ c");
    CHECK(StringFromCascadeLayerName({}).empty());
  }
}

#include "Krystal.Booey/CSS/CSSOM/CSSLayerStatementRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSLayerStatementRule", "[CSS][CSSOM][CSSLayerStatementRule]")
  {
    auto sheet = SheetFromText(u8"@layer a, b.c; @layer d;");

    auto &rule = RuleAt<CSSLayerStatementRule>(*sheet, 0);
    CHECK(TypeOf(rule) == RuleType::LayerStatement);
    auto names = rule.NameList();
    REQUIRE(names.size() == 2);
    CHECK(ToNarrow(names[0]) == "a");
    CHECK(ToNarrow(names[1]) == "b.c");
    CHECK(RuleText(rule) == "@layer a, b.c;");

    CHECK(RuleText(*sheet->Item(1)) == "@layer d;");
  }
}

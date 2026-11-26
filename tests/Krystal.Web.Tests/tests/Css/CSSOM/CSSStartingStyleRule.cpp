#include "Krystal.Web/Css/CSSOM/CSSStartingStyleRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/CSSOMTestUtils.hpp"

namespace krys::web::css::tests
{
  TEST_CASE("CSSStartingStyleRule", "[CSS][CSSOM][CSSStartingStyleRule]")
  {
    auto sheet = SheetFromText(u8"@starting-style { a { opacity: 0; } } @starting-style { }");

    auto &rule = RuleAt<CSSStartingStyleRule>(*sheet, 0);
    CHECK(TypeOf(rule) == RuleType::StartingStyle);
    CHECK(RuleText(rule) == "@starting-style {\n  a { opacity: 0; }\n}");
    CHECK(RuleText(*sheet->Item(1)) == "@starting-style {\n}");
  }
}

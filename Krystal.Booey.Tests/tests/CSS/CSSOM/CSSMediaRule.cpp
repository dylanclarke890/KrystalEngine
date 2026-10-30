#include "Krystal.Booey/CSS/CSSOM/CSSMediaRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey/CSS/Media/MediaList.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSMediaRule", "[CSS][CSSOM][CSSMediaRule]")
  {
    auto sheet = SheetFromText(u8"@media screen and (min-width: 100px), print { a { color: red; } } "
                               u8"@media all { }");
    auto &rule = RuleAt<CSSMediaRule>(*sheet, 0);

    CHECK(TypeOf(rule) == RuleType::Media);
    CHECK(Is<CSSConditionRule>(static_cast<CSSRule &>(rule)));
    CHECK(ToNarrow(static_cast<CSSConditionRule &>(rule).ConditionText())
          == "screen and (min-width: 100px), print");
    CHECK(ToNarrow(rule.Media()->MediaText()) == "screen and (min-width: 100px), print");
    CHECK(RuleText(rule) == "@media screen and (min-width: 100px), print {\n  a { color: red; }\n}");

    CHECK(RuleText(*sheet->Item(1)) == "@media all {\n}");
  }
}

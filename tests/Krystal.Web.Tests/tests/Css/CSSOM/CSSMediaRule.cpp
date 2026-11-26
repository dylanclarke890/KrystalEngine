#include "Krystal.Web/Css/CSSOM/CSSMediaRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/CSSOMTestUtils.hpp"
#include "Krystal.Web/Css/CSSOM/MediaList.hpp"

namespace krys::web::css::tests
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

  TEST_CASE("CSSMediaRule: changing its media", "[CSS][CSSOM][CSSMediaRule][cssom-1:dom-cssmediarule-media]")
  {
    auto sheet = SheetFromText(u8"@media print { a { color: red; } }");
    auto &rule = RuleAt<CSSMediaRule>(*sheet, 0);
    auto *media = rule.Media();
    REQUIRE(media);
    CHECK(media == rule.Media());
    CHECK(media->ParentRule() == &rule);

    media->AppendMedium(u8"screen");
    CHECK(ToNarrow(static_cast<CSSConditionRule &>(rule).ConditionText()) == "print, screen");
    CHECK(RuleText(rule) == "@media print, screen {\n  a { color: red; }\n}");
    CHECK(SheetText(*sheet) == "@media print, screen {\n  a { color: red; }\n}");

    // Its rules can be changed too.
    REQUIRE(rule.InsertRule(u8"b { }", 1).HasValue());
    CHECK(rule.Length() == 2u);
    CHECK(RuleText(*rule.Item(1)) == "b { }");
    CHECK(rule.Item(1)->ParentRule() == &rule);
  }
}

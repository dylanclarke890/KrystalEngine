#include "Krystal.Booey/CSS/CSSOM/CSSImportRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey/CSS/Media/MediaList.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSImportRule", "[CSS][CSSOM][CSSImportRule]")
  {
    auto sheet = SheetFromText(u8"@import \"a.css\"; "
                               u8"@import url(b.css) layer; "
                               u8"@import url(\"c.css\") layer(x.y) supports(display: grid) screen, print; "
                               u8"@import 'd\"e.css';");

    SECTION("plain")
    {
      auto &rule = RuleAt<CSSImportRule>(*sheet, 0);
      CHECK(TypeOf(rule) == RuleType::Import);
      CHECK(ToNarrow(rule.Href()) == "a.css");
      CHECK_FALSE(rule.LayerName());
      CHECK(rule.SupportsText().empty());
      CHECK(rule.Media().Length() == 0);
      CHECK(rule.GetStyleSheet() == nullptr);
      CHECK_FALSE(rule.ProtectedStyleSheet());
      CHECK(RuleText(rule) == "@import url(\"a.css\");");
    }

    SECTION("an anonymous layer")
    {
      auto &rule = RuleAt<CSSImportRule>(*sheet, 1);
      REQUIRE(rule.LayerName());
      CHECK(rule.LayerName()->empty());
      CHECK(RuleText(rule) == "@import url(\"b.css\") layer;");
    }

    SECTION("a layer, a supports condition and media queries")
    {
      auto &rule = RuleAt<CSSImportRule>(*sheet, 2);
      REQUIRE(rule.LayerName());
      CHECK(ToNarrow(*rule.LayerName()) == "x.y");
      CHECK(ToNarrow(rule.SupportsText()) == "display: grid");
      CHECK(ToNarrow(rule.Media().MediaText()) == "screen, print");
      CHECK(RuleText(rule) == "@import url(\"c.css\") layer(x.y) supports(display: grid) screen, print;");
    }

    SECTION("an escaped URL")
    {
      CHECK(RuleText(*sheet->Item(3)) == "@import url(\"d\\\"e.css\");");
    }
  }
}

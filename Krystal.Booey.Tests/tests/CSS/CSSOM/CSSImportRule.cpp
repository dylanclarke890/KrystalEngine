#include "Krystal.Booey/CSS/CSSOM/CSSImportRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey/CSS/CSSOM/MediaList.hpp"
#include "Krystal.Booey/CSS/CSSOM/StyleSheetList.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE(
    "CSSImportRule",
    "[CSS][CSSOM][CSSImportRule][cssom-1:dom-cssimportrule-href][cssom-1:dom-cssimportrule-stylesheet]")
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
      CHECK_FALSE(rule.SupportsText());
      CHECK(rule.Media().Length() == 0);
      CHECK(rule.GetStyleSheet() == nullptr);
      CHECK_FALSE(rule.GetStyleSheet());
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
      REQUIRE(rule.SupportsText());
      CHECK(ToNarrow(*rule.SupportsText()) == "display: grid");
      CHECK(ToNarrow(rule.Media().MediaText()) == "screen, print");
      CHECK(RuleText(rule) == "@import url(\"c.css\") layer(x.y) supports(display: grid) screen, print;");
    }

    SECTION("an escaped URL")
    {
      CHECK(RuleText(*sheet->Item(3)) == "@import url(\"d\\\"e.css\");");
    }
  }

  TEST_CASE("CSSImportRule: the imported style sheet",
            "[CSS][CSSOM][CSSImportRule][cssom-1:dom-cssimportrule-media]"
            "[cssom-1:dom-cssimportrule-stylesheet]")
  {
    boo::tests::LoaderFixture loader;
    loader.Strategy.AddText(u8"file:///a.css", u8"b { }");
    boo::tests::PageInFrameFixture page {u8"<style>@import 'file:///a.css' print;</style>"};
    boo::tests::RunNetworkingTasks();

    REQUIRE(page.Document->StyleSheets().Length() == 1u);
    auto &sheet = Downcast<CSSStyleSheet>(*page.Document->StyleSheets().Item(0));
    auto &rule = RuleAt<CSSImportRule>(sheet, 0);
    auto *imported = rule.GetStyleSheet();
    REQUIRE(imported);
    CHECK(imported == rule.GetStyleSheet());
    CHECK(imported->OwnerRule() == &rule);
    CHECK(imported->ParentStyleSheet() == &sheet);
    CHECK(imported->Length() == 1u);

    // The rule's media is its style sheet's.
    CHECK(imported->Media() == &rule.Media());
    CHECK(ToNarrow(imported->Media()->MediaText()) == "print");
    imported->Media()->AppendMedium(u8"screen");
    CHECK(RuleText(rule) == "@import url(\"file:///a.css\") print, screen;");
  }
}

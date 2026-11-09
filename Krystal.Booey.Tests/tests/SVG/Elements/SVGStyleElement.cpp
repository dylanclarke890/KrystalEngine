#include "Krystal.Booey/SVG/Elements/SVGStyleElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRuleList.hpp"
#include "Krystal.Booey/CSS/Media/MediaList.hpp"
#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetList.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using css::tests::ToNarrow;
  using style::tests::ResolverFixture;

  TEST_CASE("SVGStyleElement: an error event when an import fails, and no load event",
            "[SVG][SVGStyleElement]")
  {
    boo::tests::LoaderFixture loader;
    loader.Strategy.AddText(u8"file:///ok.css", u8"rect { fill: green }");
    boo::tests::PageInFrameFixture fixture(u8"<svg id=s></svg>");
    auto &svg = fixture.ById(u8"s");

    // As Chrome.
    auto add = [&](utf8_stringview text)
    {
      auto style = CreateRef<SVGStyleElement>(*fixture.Document);
      REQUIRE_FALSE(style->TextContent(dom::DOMString(text)).HasException());
      auto events = boo::tests::LoadEvents::ListenTo(*style);
      REQUIRE_FALSE(svg.AppendChild(*style).HasException());
      return events;
    };
    auto plain = add(u8"rect {}");
    auto imported = add(u8"@import url(file:///ok.css);");
    auto missing = add(u8"@import url(file:///missing.css);");
    boo::tests::RunNetworkingTasks();

    CHECK(plain->Loads == 0);
    CHECK(plain->Errors == 0);
    CHECK(imported->Loads == 0);
    CHECK(imported->Errors == 0);
    CHECK(missing->Loads == 0);
    CHECK(missing->Errors == 1);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGStyleElement: the style sheet", "[SVG][SVGStyleElement]")
  {
    auto document = ResolverFixture::Parse(u8"<!DOCTYPE html><body><svg id=s></svg>");
    auto &svg = *document->GetElementById(u8"s");
    auto style = CreateRef<SVGStyleElement>(*document);
    REQUIRE_FALSE(style->TextContent(u8"a {} b {}").HasException());
    CHECK(style->Sheet() == nullptr);

    REQUIRE_FALSE(svg.AppendChild(*style).HasException());
    auto *sheet = style->Sheet();
    REQUIRE(sheet != nullptr);
    CHECK(sheet->OwnerNode() == style.get());
    CHECK(sheet->CssRules()->Length() == 2u);
    REQUIRE(document->StyleSheets().Length() == 1u);
    CHECK(document->StyleSheets().Item(0) == sheet);
    CHECK(document->StyleScope().ActiveStyleSheetsContains(*sheet));

    SECTION("title")
    {
      style->Title(u8"one");
      CHECK(style->GetAttribute(u8"title") == u8"one");
      CHECK(style->Title() == u8"one");
      CHECK(ToNarrow(style->Sheet()->Title()) == "one");
    }

    SECTION("media")
    {
      style->Media(u8"print");
      CHECK(style->Media() == u8"print");
      CHECK(ToNarrow(style->Sheet()->Media()->MediaText()) == "print");
      CHECK(document->StyleScope().HasPendingUpdate());
    }

    SECTION("type")
    {
      style->Type(u8"text/plain");
      CHECK(style->Type() == u8"text/plain");
      CHECK(style->Sheet() == nullptr);
      CHECK(document->StyleSheets().Length() == 0u);

      style->Type(u8"text/css");
      CHECK(style->Sheet() != nullptr);
      CHECK(document->StyleSheets().Length() == 1u);
    }

    SECTION("disabled")
    {
      CHECK_FALSE(style->Disabled());
      style->Disabled(true);
      CHECK(style->Disabled());
      CHECK(style->Sheet()->Disabled());
      document->StyleScope().FlushPendingUpdate();
      CHECK_FALSE(document->StyleScope().ActiveStyleSheetsContains(*style->Sheet()));
    }

    SECTION("Removing the element removes its sheet")
    {
      auto removed = ShareRefPtr(sheet);
      REQUIRE_FALSE(style->Remove().HasException());
      CHECK(style->Sheet() == nullptr);
      CHECK(removed->OwnerNode() == nullptr);
      CHECK(document->StyleSheets().Length() == 0u);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGStyleElement: its rules apply", "[SVG][SVGStyleElement]")
  {
    ResolverFixture fixture {u8"<!DOCTYPE html><body><p id=p></p><svg id=s></svg></body>"};
    auto style = CreateRef<SVGStyleElement>(*fixture.Document);
    REQUIRE_FALSE(style->TextContent(u8"p { margin-left: 7px }").HasException());
    REQUIRE_FALSE(fixture.ById(u8"s").AppendChild(*style).HasException());
    fixture.Document->StyleScope().FlushPendingUpdate();

    auto resolved = fixture.Resolve(fixture.ById(u8"p"));
    CHECK(ResolverFixture::Serialised(*resolved.Style, resolved.Style->MarginLeft()) == "7px");
  }
}

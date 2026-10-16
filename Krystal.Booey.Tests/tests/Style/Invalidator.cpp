#include "Krystal.Booey/Style/Invalidator.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/HTMLStyleElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    const boo::Color Blue {boo::Color::blue};
    const boo::Color Red {boo::Color::red};

    KRYS_NODISCARD Ref<dom::Element> Create(dom::Document &document, dom::DOMStringAtom localName)
    {
      auto element = document.CreateElement(localName);
      REQUIRE_FALSE(element.HasException());
      return element.ReleaseValue();
    }

    KRYS_NODISCARD const RenderStyle &Rendered(const dom::Element &element)
    {
      auto *style = element.GetRenderStyle();
      REQUIRE(style);
      return *style;
    }

    KRYS_NODISCARD Ref<html::HTMLStyleElement> CreateStyle(dom::Document &document, dom::DOMString &&text)
    {
      auto style = CreateRef<html::HTMLStyleElement>(document);
      REQUIRE_FALSE(style->TextContent(krys::move(text)).HasException());
      return style;
    }

    KRYS_NODISCARD Ref<css::StyleSheetContents> ParseSheet(utf8_string &&text)
    {
      auto contents = css::StyleSheetContents::Create(css::ParserContext(css::ParserMode::HTMLStandard));
      contents->ParseString(text);
      return contents;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Invalidator: style sheet changes", "[Style][Invalidator]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>p { color: rgb(255, 0, 0) }</style>
      <body><p id=a class=x></p><p id=b></p></body>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    auto &a = fixture.ById(u8"a");
    auto &b = fixture.ById(u8"b");
    REQUIRE(Rendered(a).GetColor() == Red);

    SECTION("An added style sheet invalidates the elements its rules match")
    {
      auto style = CreateStyle(document, u8".x { color: rgb(0, 0, 255) }");
      REQUIRE_FALSE(document.Body()->AppendChild(*style).HasException());
      document.StyleScope().FlushPendingUpdate();

      CHECK_FALSE(document.HasPendingFullStyleRebuild());
      CHECK(a.StyleValidity() == Validity::ElementInvalid);
      CHECK(b.StyleValidity() == Validity::Valid);

      document.UpdateStyleIfNeeded();
      CHECK(Rendered(a).GetColor() == Blue);
      CHECK(Rendered(b).GetColor() == Red);

      SECTION("A removed one rebuilds all style")
      {
        REQUIRE_FALSE(document.Body()->RemoveChild(*style).HasException());
        document.StyleScope().FlushPendingUpdate();
        CHECK(document.HasPendingFullStyleRebuild());

        document.UpdateStyleIfNeeded();
        CHECK(Rendered(a).GetColor() == Red);
      }
    }

    SECTION("Rules other than style rules rebuild all style")
    {
      auto style =
        CreateStyle(document, u8"@font-face { font-family: f; src: local(f) } .x { color: rgb(0, 0, 255) }");
      REQUIRE_FALSE(document.Body()->AppendChild(*style).HasException());
      document.StyleScope().FlushPendingUpdate();
      CHECK(document.HasPendingFullStyleRebuild());

      document.UpdateStyleIfNeeded();
      CHECK(Rendered(a).GetColor() == Blue);
    }

    SECTION("Media and nested style rules are looked into")
    {
      auto style =
        CreateStyle(document, u8"@media all { .x { color: rgb(0, 0, 255) } } #b { & { color: blue } }");
      REQUIRE_FALSE(document.Body()->AppendChild(*style).HasException());
      document.StyleScope().FlushPendingUpdate();
      CHECK_FALSE(document.HasPendingFullStyleRebuild());
      CHECK(a.StyleValidity() == Validity::ElementInvalid);
      CHECK(b.StyleValidity() == Validity::ElementInvalid);

      document.UpdateStyleIfNeeded();
      CHECK(Rendered(a).GetColor() == Blue);
      CHECK(Rendered(b).GetColor() == Blue);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Invalidator: rule information and direct invalidation",
                   "[Style][Invalidator]")
  {
    ResolverFixture fixture {
      u8R"(<!DOCTYPE html><body><div id=d class=x><p id=p class=y></p><p id=q></p></div>
      <div id=e></div></body>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    SECTION("Sheets with only style rules don't dirty all style")
    {
      Invalidator invalidator({ParseSheet(u8".y { color: blue }")},
                              fixture.GetResolver().MediaQueryEvaluator());
      CHECK_FALSE(invalidator.DirtiesAllStyle());

      invalidator.InvalidateStyle(document);
      CHECK(fixture.ById(u8"p").StyleValidity() == Validity::ElementInvalid);
      CHECK(fixture.ById(u8"q").StyleValidity() == Validity::Valid);
      CHECK(fixture.ById(u8"d").StyleValidity() == Validity::Valid);
    }

    SECTION("Other rules dirty all style")
    {
      Invalidator invalidator({ParseSheet(u8"@page { margin: 0 }")},
                              fixture.GetResolver().MediaQueryEvaluator());
      CHECK(invalidator.DirtiesAllStyle());

      invalidator.InvalidateStyle(document.StyleScope());
      CHECK(document.HasPendingFullStyleRebuild());
    }

    SECTION("Invalidating an element's subtree")
    {
      Invalidator invalidator({ParseSheet(u8".x, .y { color: blue }")},
                              fixture.GetResolver().MediaQueryEvaluator());
      invalidator.InvalidateStyle(fixture.ById(u8"d"));
      CHECK(fixture.ById(u8"d").StyleValidity() == Validity::ElementInvalid);
      CHECK(fixture.ById(u8"p").StyleValidity() == Validity::ElementInvalid);
      CHECK(fixture.ById(u8"e").StyleValidity() == Validity::Valid);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Invalidator: shadow trees", "[Style][Invalidator]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body><div id=host><span id=light class=s></span></div></body>)html"};
    auto &document = *fixture.Document;
    auto &host = fixture.ById(u8"host");
    auto shadow = host.AttachShadow({}).ReleaseValue();
    auto inner = Create(document, u8"div");
    REQUIRE_FALSE(inner->SetAttribute(u8"class", u8"i").HasException());
    auto slot = Create(document, u8"slot");
    REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());
    REQUIRE_FALSE(shadow->AppendChild(*slot).HasException());
    document.UpdateStyleIfNeeded();
    REQUIRE(Rendered(*inner).GetColor() != Blue);

    SECTION("An added sheet invalidates its shadow tree")
    {
      auto style = CreateStyle(document, u8".i { color: rgb(0, 0, 255) }");
      REQUIRE_FALSE(shadow->AppendChild(*style).HasException());
      document.UpdateStyleIfNeeded();
      CHECK(Rendered(*inner).GetColor() == Blue);
    }

    SECTION(":host and ::slotted() rules invalidate the host and its children")
    {
      auto style = CreateStyle(document, u8":host { color: rgb(0, 0, 255) } ::slotted(.s) { width: 5px }");
      REQUIRE_FALSE(shadow->AppendChild(*style).HasException());
      document.UpdateStyleIfNeeded();
      CHECK(Rendered(host).GetColor() == Blue);
      CHECK(ResolverFixture::Serialised(Rendered(fixture.ById(u8"light")),
                                        Rendered(fixture.ById(u8"light")).Width())
            == "5px");

      Invalidator::InvalidateHostAndSlottedStyleIfNeeded(*shadow);
      CHECK(host.StyleValidity() == Validity::ElementInvalid);
      CHECK(fixture.ById(u8"light").StyleValidity() == Validity::ElementInvalid);
      document.UpdateStyleIfNeeded();

      // Changing the sheet's contents invalidates them again.
      REQUIRE_FALSE(style->TextContent(u8".i { color: rgb(0, 0, 255) }").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(Rendered(host).GetColor() != Blue);
      CHECK(ResolverFixture::Serialised(Rendered(fixture.ById(u8"light")),
                                        Rendered(fixture.ById(u8"light")).Width())
            == "auto");
    }

    SECTION("::part() rules")
    {
      REQUIRE_FALSE(inner->SetAttribute(u8"part", u8"p").HasException());
      auto style = CreateStyle(document, u8"#host::part(p) { color: rgb(0, 0, 255) }");
      REQUIRE_FALSE(document.Body()->AppendChild(*style).HasException());
      document.UpdateStyleIfNeeded();
      CHECK(Rendered(*inner).GetColor() == Blue);

      // Changing the part name invalidates the element.
      REQUIRE_FALSE(inner->SetAttribute(u8"part", u8"q").HasException());
      CHECK(inner->StyleValidity() == Validity::ElementInvalid);
      document.UpdateStyleIfNeeded();
      CHECK(Rendered(*inner).GetColor() != Blue);

      Invalidator::InvalidateShadowParts(*shadow);
      CHECK(inner->StyleValidity() == Validity::ElementInvalid);
    }
  }
}

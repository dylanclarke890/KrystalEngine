#include "Krystal.Booey/HTML/HTMLLinkElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE("HTMLLinkElement::RelList", "[HTML][HTMLLinkElement]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto link = CreateRef<HTMLLinkElement>(*document);
    REQUIRE_FALSE(link->SetAttribute(u8"rel", u8"stylesheet icon").HasException());

    auto &relList = link->RelList();
    REQUIRE(relList.Length() == 2uz);
    REQUIRE(relList.Contains(u8"icon"));

    REQUIRE_FALSE(relList.Remove(dom::DOMString(u8"icon")).HasException());
    REQUIRE(link->GetAttribute(u8"rel") == u8"stylesheet");
  }

  namespace
  {
    using boo::tests::LoaderFixture;
    using boo::tests::LoadEvents;
    using boo::tests::PageInFrameFixture;
    using boo::tests::RunNetworkingTasks;

    KRYS_NODISCARD bool HasRenderer(PageInFrameFixture &fixture, dom::DOMStringView id) noexcept
    {
      (void)fixture.Document->UpdateLayout();
      return fixture.ById(id).Renderer() != nullptr;
    }

    KRYS_NODISCARD HTMLLinkElement &Link(PageInFrameFixture &fixture, dom::DOMStringView id) noexcept
    {
      return Downcast<HTMLLinkElement>(fixture.ById(id));
    }
  }

  TEST_CASE("HTMLLinkElement: a linked sheet loads, holds back rendering until it does, and fires load",
            "[HTML][HTMLLinkElement]")
  {
    LoaderFixture loader;
    loader.Strategy.AddText(u8"file:///css/a.css", u8"#box { width: 30px; height: 20px }");
    loader.Strategy.Defer = true;

    PageInFrameFixture fixture(u8"<head><link id=link rel=stylesheet href='file:///css/a.css'></head>"
                               u8"<body><div id=box></div></body>");
    auto &link = Link(fixture, u8"link");
    auto events = LoadEvents::ListenTo(link);

    REQUIRE(loader.Strategy.Requests == List<utf8_string> {u8"file:///css/a.css"});
    CHECK(link.StyleSheetIsLoading());
    CHECK_FALSE(link.Sheet());
    // The page waits for the sheet in its head: nothing has a renderer.
    CHECK(fixture.Document->StyleScope().HasPendingSheetsBeforeBody());
    CHECK_FALSE(HasRenderer(fixture, u8"box"));

    loader.Strategy.CompleteDeferred();
    RunNetworkingTasks();

    CHECK(events->Loads == 1);
    CHECK(events->Errors == 0);
    CHECK_FALSE(link.StyleSheetIsLoading());
    REQUIRE(link.Sheet());
    CHECK_FALSE(fixture.Document->StyleScope().HasPendingSheets());
    CHECK(fixture.RendererSize(u8"box") == IntSize(30, 20));
  }

  TEST_CASE("HTMLLinkElement: imports load, nested and with media conditions", "[HTML][HTMLLinkElement]")
  {
    LoaderFixture loader;
    loader.Strategy.AddText(u8"file:///css/a.css",
                            u8"@import url(b.css); @import 'print.css' print; #box { height: 10px }");
    loader.Strategy.AddText(u8"file:///css/b.css", u8"@import 'deep/c.css'; #box { width: 40px }");
    loader.Strategy.AddText(u8"file:///css/deep/c.css", u8"#box { padding-left: 2px }");
    loader.Strategy.AddText(u8"file:///css/print.css", u8"#box { width: 99px }");

    PageInFrameFixture fixture(u8"<head><link id=link rel=stylesheet href='file:///css/a.css'></head>"
                               u8"<body><div id=box style='display: inline-block'></div></body>");
    auto events = LoadEvents::ListenTo(fixture.ById(u8"link"));
    RunNetworkingTasks();

    CHECK(events->Loads == 1);
    // Imports resolve against their sheet's URL; a print import loads but doesn't apply.
    CHECK(std::ranges::find(loader.Strategy.Requests, u8"file:///css/deep/c.css")
          != loader.Strategy.Requests.end());
    CHECK(std::ranges::find(loader.Strategy.Requests, u8"file:///css/print.css")
          != loader.Strategy.Requests.end());
    CHECK(fixture.RendererSize(u8"box") == IntSize(42, 10));
  }

  TEST_CASE("HTMLLinkElement: a sheet's byte order mark is dropped", "[HTML][HTMLLinkElement]")
  {
    LoaderFixture loader;
    // Its first rule is an import, which must still come first.
    loader.Strategy.AddText(u8"file:///css/a.css", u8"\uFEFF@import url(b.css); #box { height: 10px }");
    loader.Strategy.AddText(u8"file:///css/b.css", u8"\uFEFF#box { width: 40px }");

    PageInFrameFixture fixture(u8"<head><link rel=stylesheet href='file:///css/a.css'></head>"
                               u8"<body><div id=box style='display: inline-block'></div></body>");
    RunNetworkingTasks();

    CHECK(fixture.RendererSize(u8"box") == IntSize(40, 10));
  }

  TEST_CASE("HTMLLinkElement: a cycle of imports ends", "[HTML][HTMLLinkElement]")
  {
    LoaderFixture loader;
    loader.Strategy.AddText(u8"file:///a.css", u8"@import 'b.css'; #box { width: 10px }");
    loader.Strategy.AddText(u8"file:///b.css", u8"@import 'a.css'; #box { height: 10px }");

    PageInFrameFixture fixture(u8"<link id=link rel=stylesheet href='file:///a.css'><div id=box></div>");
    auto events = LoadEvents::ListenTo(fixture.ById(u8"link"));
    RunNetworkingTasks();

    CHECK(events->Loads == 1);
    CHECK(loader.Strategy.Requests.size() == 2u);
    CHECK(fixture.RendererSize(u8"box") == IntSize(10, 10));
  }

  TEST_CASE("HTMLLinkElement: failed sheets and sheets that aren't CSS fire error", "[HTML][HTMLLinkElement]")
  {
    LoaderFixture loader;
    loader.Strategy.AddText(u8"file:///text.css", u8"#box { width: 10px }", u8"text/plain");
    loader.Strategy.AddText(u8"file:///imports.css", u8"@import 'missing.css'; #box { height: 10px }");

    PageInFrameFixture fixture(u8"<head><link id=missing rel=stylesheet href='file:///missing.css'>"
                               u8"<link id=text rel=stylesheet href='file:///text.css'>"
                               u8"<link id=imports rel=stylesheet href='file:///imports.css'></head>"
                               u8"<body><div id=box style='width: 5px'></div></body>");
    auto missing = LoadEvents::ListenTo(fixture.ById(u8"missing"));
    auto text = LoadEvents::ListenTo(fixture.ById(u8"text"));
    auto imports = LoadEvents::ListenTo(fixture.ById(u8"imports"));
    RunNetworkingTasks();

    CHECK(missing->Errors == 1);
    CHECK(text->Errors == 1);
    // A sheet whose import failed still applies, but reports an error.
    CHECK(imports->Errors == 1);
    // The page doesn't wait for them any more.
    CHECK(fixture.RendererSize(u8"box") == IntSize(5, 10));
  }

  TEST_CASE("HTMLLinkElement: media, disabled sheets, and links added and removed", "[HTML][HTMLLinkElement]")
  {
    LoaderFixture loader;
    loader.Strategy.AddText(u8"file:///print.css", u8"#box { width: 99px }");
    loader.Strategy.AddText(u8"file:///wide.css", u8"#box { width: 50px }");
    loader.Strategy.Defer = true;

    PageInFrameFixture fixture(u8"<head><link id=print rel=stylesheet media=print href='file:///print.css'>"
                               u8"<link id=disabled rel=stylesheet disabled href='file:///wide.css'></head>"
                               u8"<body><div id=box style='height: 5px'></div></body>");
    // A sheet whose media doesn't match loads without holding back rendering; a disabled one doesn't load.
    CHECK(loader.Strategy.Requests == List<utf8_string> {u8"file:///print.css"});
    CHECK(HasRenderer(fixture, u8"box"));

    loader.Strategy.CompleteDeferred();
    RunNetworkingTasks();
    CHECK(fixture.RendererSize(u8"box") == IntSize(184, 5));

    // Enabling the sheet loads it.
    loader.Strategy.Defer = false;
    auto &disabled = Link(fixture, u8"disabled");
    disabled.RemoveAttribute(u8"disabled");
    RunNetworkingTasks();
    REQUIRE(disabled.Sheet());
    CHECK(fixture.RendererSize(u8"box") == IntSize(50, 5));

    // Removing it removes its styles; adding a link loads its sheet (from the memory cache).
    REQUIRE_FALSE(disabled.Remove().HasException());
    CHECK(fixture.RendererSize(u8"box") == IntSize(184, 5));
    auto link = fixture.Document->CreateElement(u8"link").ReleaseValue();
    REQUIRE_FALSE(link->SetAttribute(u8"rel", u8"stylesheet").HasException());
    REQUIRE_FALSE(link->SetAttribute(u8"href", u8"file:///wide.css").HasException());
    REQUIRE_FALSE(fixture.Document->Body()->AppendChild(*link).HasException());
    RunNetworkingTasks();
    CHECK(fixture.RendererSize(u8"box") == IntSize(50, 5));
    CHECK(loader.Strategy.Requests.size() == 2u);
  }
}

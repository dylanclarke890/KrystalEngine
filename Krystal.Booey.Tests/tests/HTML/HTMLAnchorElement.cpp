#include "Krystal.Booey/HTML/HTMLAnchorElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/HTML/HTMLAreaElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::NavigationFixture;
  using boo::tests::Str;

  namespace
  {
    /// @brief Dispatches a trusted-looking click of @p button at @p target, with Ctrl held if @p ctrlKey.
    void Click(dom::Element &target, int16 button = 0, bool ctrlKey = false) noexcept
    {
      dom::MouseEventInit init;
      init.Bubbles = true;
      init.Cancelable = true;
      init.Composed = true;
      init.Button = button;
      init.CtrlKey = ctrlKey;
      auto type = button == 0 ? dom::DOMStringAtom(u8"click") : dom::DOMStringAtom(u8"auxclick");
      (void)target.DispatchEvent(*CreateRef<dom::MouseEvent>(type, init));
    }
  }

  TEST_CASE("HTMLAnchorElement: a click follows the link", "[HTML][HTMLAnchorElement]")
  {
    NavigationFixture fixture(
      u8R"(<a id=a href="next.html"><span id=inner>Next</span></a><a id=plain>No href</a>
      <a id=blank href="https://other.test/" target=_blank>Blank</a>
      <a id=named href="x.html" target=results rel="noopener">Named</a>
      <a id=dl href="file.zip" download="saved.zip" target=_blank>Download</a>)");

    SECTION("the host is asked, with the action")
    {
      Click(fixture.ById(u8"inner"), 0, true);
      REQUIRE(fixture.Navigations().size() == 1);
      auto &navigation = fixture.Navigations()[0];
      CHECK(navigation.URL == "https://example.com/dir/next.html");
      CHECK(navigation.HTTPMethod == "GET");
      CHECK_FALSE(navigation.IsNewWindow);
      CHECK(navigation.Action.Type() == NavigationType::LinkClicked);
      CHECK(navigation.Action.SourceElement() == &fixture.ById(u8"a"));
      REQUIRE(navigation.Action.GetMouseEventData());
      CHECK(navigation.Action.GetMouseEventData()->Button == MouseButton::Left);
      REQUIRE(navigation.Action.KeyStateEventData());
      CHECK(navigation.Action.KeyStateEventData()->CtrlKey);
      CHECK_FALSE(navigation.Action.KeyStateEventData()->ShiftKey);
      // Another document isn't loaded yet, so the page stays.
      CHECK(fixture.DocumentURL() == "https://example.com/dir/page.html");
    }

    SECTION("a link without href isn't followed")
    {
      Click(fixture.ById(u8"plain"));
      CHECK(fixture.Navigations().empty());
    }

    SECTION("a canceled click isn't followed")
    {
      auto handle =
        fixture.ById(u8"a").AddEventListener(u8"click", [](dom::Event &event) { event.PreventDefault(); });
      Click(fixture.ById(u8"inner"));
      CHECK(fixture.Navigations().empty());
    }

    SECTION("a target naming another browsing context asks for a new window")
    {
      Click(fixture.ById(u8"blank"));
      REQUIRE(fixture.Navigations().size() == 1);
      CHECK(fixture.Navigations()[0].IsNewWindow);
      CHECK(fixture.Navigations()[0].NewWindowName == "_blank");
      // `_blank` gets no opener unless the link asks for one.
      CHECK(fixture.Navigations()[0].Action.GetNewFrameOpenerPolicy() == NewFrameOpenerPolicy::Suppress);

      Click(fixture.ById(u8"named"));
      REQUIRE(fixture.Navigations().size() == 2);
      CHECK(fixture.Navigations()[1].NewWindowName == "results");
      CHECK(fixture.Navigations()[1].Action.GetNewFrameOpenerPolicy() == NewFrameOpenerPolicy::Suppress);
    }

    SECTION("a download ignores the target")
    {
      Click(fixture.ById(u8"dl"));
      REQUIRE(fixture.Navigations().size() == 1);
      CHECK_FALSE(fixture.Navigations()[0].IsNewWindow);
      CHECK(fixture.Navigations()[0].Action.DownloadAttribute() == dom::DOMStringAtom(u8"saved.zip"));
    }

    SECTION("the document's base target applies to links without one")
    {
      auto base = fixture.Document->CreateElement(u8"base").ReleaseValue();
      (void)base->SetAttribute(u8"target", u8"side");
      REQUIRE_FALSE(fixture.ById(u8"a").ParentNode()->AppendChild(*base).HasException());
      Click(fixture.ById(u8"inner"));
      REQUIRE(fixture.Navigations().size() == 1);
      CHECK(fixture.Navigations()[0].NewWindowName == "side");
    }

    SECTION("the middle button follows it too, and the host sees which button")
    {
      Click(fixture.ById(u8"inner"), 1);
      REQUIRE(fixture.Navigations().size() == 1);
      CHECK(fixture.Navigations()[0].Action.GetMouseEventData()->Button == MouseButton::Middle);
      // The right button doesn't.
      Click(fixture.ById(u8"inner"), 2);
      CHECK(fixture.Navigations().size() == 1);
    }
  }

  TEST_CASE("HTMLAnchorElement: Enter follows a focused link", "[HTML][HTMLAnchorElement]")
  {
    NavigationFixture fixture(u8R"(<a id=a href="#section">Section</a>)");
    auto &a = fixture.Get<HTMLAnchorElement>(u8"a");
    (void)fixture.Document->UpdateLayoutIgnorePendingStylesheets();
    a.Focus();
    REQUIRE(a.Focused());

    dom::KeyboardEventInit init;
    init.Bubbles = true;
    init.Cancelable = true;
    init.KeyIdentifier = u8"Enter";
    init.ShiftKey = true;
    (void)a.DispatchEvent(*CreateRef<dom::KeyboardEvent>(dom::DOMStringAtom(u8"keydown"), init));

    REQUIRE(fixture.Navigations().size() == 1);
    auto &navigation = fixture.Navigations()[0];
    CHECK(navigation.URL == "https://example.com/dir/page.html#section");
    // The simulated click's underlying key press has the modifiers.
    REQUIRE(navigation.Action.KeyStateEventData());
    CHECK(navigation.Action.KeyStateEventData()->ShiftKey);
  }

  TEST_CASE("HTMLAreaElement: a click follows the link", "[HTML][HTMLAreaElement]")
  {
    NavigationFixture fixture(
      u8R"(<map name=m><area id=area href="area.html" target=_self><area id=nohref></map>)");
    Click(fixture.ById(u8"area"));
    REQUIRE(fixture.Navigations().size() == 1);
    CHECK(fixture.Navigations()[0].URL == "https://example.com/dir/area.html");
    CHECK_FALSE(fixture.Navigations()[0].IsNewWindow);

    Click(fixture.ById(u8"nohref"));
    CHECK(fixture.Navigations().size() == 1);
  }

  TEST_CASE("HTMLAnchorElement: the rest of its interface", "[HTML][HTMLAnchorElement]")
  {
    NavigationFixture fixture(
      u8R"(<a id=a href="x" target=t hreflang=fr type="text/html" rel="noreferrer">A</a>
      <a id=b>B</a><img id=img src="i.png">)");
    auto &a = fixture.Get<HTMLAnchorElement>(u8"a");
    CHECK(Str(a.Target()) == "t");
    CHECK(Str(a.HrefLang()) == "fr");
    CHECK(Str(a.Type()) == "text/html");
    CHECK(a.IsInteractiveContent());
    CHECK(a.Draggable());

    auto &b = fixture.Get<HTMLAnchorElement>(u8"b");
    CHECK_FALSE(b.IsInteractiveContent());
    CHECK_FALSE(b.Draggable());
    CHECK(fixture.Get<HTMLImageElement>(u8"img").Draggable());

    b.Target(u8"_top");
    CHECK(b.GetAttribute(u8"target") == u8"_top");
  }

  TEST_CASE("HTMLImageElement::IsServerMap", "[HTML][HTMLImageElement]")
  {
    NavigationFixture fixture(u8R"(<img id=plain><img id=ismap ismap><img id=client ismap usemap="#m">
      <img id=empty ismap usemap="">)");
    CHECK_FALSE(fixture.Get<HTMLImageElement>(u8"plain").IsServerMap());
    CHECK(fixture.Get<HTMLImageElement>(u8"ismap").IsServerMap());
    CHECK_FALSE(fixture.Get<HTMLImageElement>(u8"client").IsServerMap());
    CHECK_FALSE(fixture.Get<HTMLImageElement>(u8"empty").IsServerMap());
  }
}

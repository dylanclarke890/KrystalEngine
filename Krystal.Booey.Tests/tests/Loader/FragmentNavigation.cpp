#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/HighlightRegistry.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HashChangeEvent.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/URL/URL.hpp"

namespace krys::boo::tests
{
  namespace
  {
    constexpr utf8_stringview Page = u8R"(<style>body { margin: 0 } p, h2 { margin: 0; height: 20px }
      .spacer { height: 1000px } h2:target { height: 40px }</style><a id=link href="#section">Go</a><div class=spacer id=first></div>
      <h2 id=section>Section</h2><div class=spacer></div><a name=named>Named</a><div class=spacer></div>
      <p id="café">Café</p><div class=spacer></div><button id=button>Button</button><div class=spacer></div>)";

    struct FragmentFixture : NavigationFixture
    {
      FragmentFixture() noexcept : NavigationFixture(Page)
      {
        (void)Document->UpdateLayoutIgnorePendingStylesheets();
      }

      void Navigate(utf8_stringview url) noexcept
      {
        Frame->Loader().ChangeLocation(url::URL(Document->GetURL(), url), dom::DOMStringAtom::Empty(),
                                       nullptr, ReferrerPolicy::EmptyString, std::nullopt,
                                       dom::DOMStringAtom::Null());
        html::EventLoop::Current().RunUntilIdle();
      }

      KRYS_NODISCARD int32 ScrollY() const noexcept
      {
        return Frame->View()->GetScrollPosition().Y();
      }

      /// @brief How far below the top of the viewport @p id's element is.
      KRYS_NODISCARD double TopOf(dom::DOMStringView id) noexcept
      {
        (void)Document->UpdateLayoutIgnorePendingStylesheets();
        return ById(id).GetBoundingClientRect()->Y();
      }

      KRYS_NODISCARD dom::Element *Target() const noexcept
      {
        return Document->CSSTarget();
      }
    };
  }

  TEST_CASE("Document::FindAnchor", "[Loader][FragmentNavigation]")
  {
    NavigationFixture fixture(
      u8"<a name=named>A</a><p id=id>P</p><a id=both name=id>B</a><a name=Mixed>M</a>");
    auto &document = *fixture.Document;
    CHECK(document.FindAnchor(u8"") == nullptr);
    CHECK(document.FindAnchor(u8"id") == &fixture.ById(u8"id"));
    CHECK(document.FindAnchor(u8"named") == document.QuerySelector(u8"[name=named]").Value().get());
    // Names match exactly outside quirks mode.
    CHECK(document.FindAnchor(u8"mixed") == nullptr);
    CHECK(document.FindAnchor(u8"missing") == nullptr);
  }

  TEST_CASE("Fragment navigation", "[Loader][FragmentNavigation]")
  {
    FragmentFixture fixture;
    List<std::pair<std::string, std::string>> hashChanges;
    auto handle = fixture.Document->AddEventListener<html::HashChangeEvent>(
      u8"hashchange", [&](html::HashChangeEvent &event)
      { hashChanges.emplace_back(Str(event.OldURL()), Str(event.NewURL())); });

    SECTION("a link to a fragment scrolls its target into view and makes it the target")
    {
      fixture.ById(u8"link").DispatchSimulatedClick(nullptr);
      html::EventLoop::Current().RunUntilIdle();

      CHECK(fixture.DocumentURL() == "https://example.com/dir/page.html#section");
      CHECK(fixture.Target() == &fixture.ById(u8"section"));
      CHECK(fixture.Document->QuerySelector(u8":target").Value().get() == &fixture.ById(u8"section"));
      CHECK(fixture.ScrollY() > 0);
      CHECK(fixture.TopOf(u8"section") == 0);
      // Its :target style applies.
      CHECK(fixture.ById(u8"section").GetBoundingClientRect()->Height() == 40);
      // The heading isn't focusable: the viewport is focused, and navigation starts from the heading.
      CHECK(fixture.Document->FocusedElement() == nullptr);
      CHECK(fixture.Document->SequentialFocusNavigationStartingPoint() == &fixture.ById(u8"section"));

      REQUIRE(hashChanges.size() == 1);
      CHECK(hashChanges[0].first == "https://example.com/dir/page.html");
      CHECK(hashChanges[0].second == "https://example.com/dir/page.html#section");
    }

    SECTION("a focusable target is focused")
    {
      fixture.Navigate(u8"#button");
      CHECK(fixture.Document->FocusedElement() == &fixture.ById(u8"button"));
      CHECK(fixture.TopOf(u8"button") == 0);
    }

    SECTION("#top and an empty fragment scroll to the top, with no target")
    {
      fixture.Navigate(u8"#section");
      REQUIRE(fixture.ScrollY() > 0);
      fixture.Navigate(u8"#top");
      CHECK(fixture.ScrollY() == 0);
      CHECK(fixture.Target() == nullptr);
      (void)fixture.Document->UpdateLayoutIgnorePendingStylesheets();
      CHECK(fixture.ById(u8"section").GetBoundingClientRect()->Height() == 20);

      fixture.Navigate(u8"#section");
      fixture.Navigate(u8"#");
      CHECK(fixture.ScrollY() == 0);
      CHECK(hashChanges.size() == 4);
    }

    SECTION("names and percent-encoded IDs")
    {
      fixture.Navigate(u8"#named");
      CHECK(fixture.Target() == fixture.Document->QuerySelector(u8"[name=named]").Value().get());
      fixture.Navigate(u8"#caf%C3%A9");
      CHECK(fixture.Target() == &fixture.ById(u8"café"));
      CHECK(fixture.TopOf(u8"café") == 0);
    }

    SECTION("a fragment that indicates nothing changes the URL but not the scroll position")
    {
      fixture.Navigate(u8"#section");
      auto scrollY = fixture.ScrollY();
      fixture.Navigate(u8"#nothing");
      CHECK(fixture.DocumentURL() == "https://example.com/dir/page.html#nothing");
      CHECK(fixture.Target() == nullptr);
      CHECK(fixture.ScrollY() == scrollY);
      CHECK(hashChanges.size() == 2);
    }

    SECTION("the same fragment again scrolls again, without a hashchange")
    {
      fixture.Navigate(u8"#section");
      fixture.Frame->View()->SetScrollPosition({0, 0});
      fixture.Navigate(u8"#section");
      CHECK(fixture.TopOf(u8"section") == 0);
      CHECK(hashChanges.size() == 1);
    }
  }

  TEST_CASE("Fragment navigation keeps the target in view", "[Loader][FragmentNavigation]")
  {
    FragmentFixture fixture;
    fixture.Navigate(u8"#section");
    REQUIRE(fixture.TopOf(u8"section") == 0);
    CHECK(fixture.Frame->View()->MaintainedScrollPositionAnchor() == &fixture.ById(u8"section"));

    // Content above the target grows: layout puts the target back at the top.
    (void)fixture.ById(u8"first").SetAttribute(u8"style", u8"height: 1500px");
    (void)fixture.Document->UpdateLayoutIgnorePendingStylesheets();
    html::EventLoop::Current().RunUntilIdle();
    CHECK(fixture.TopOf(u8"section") == 0);

    // Until the user scrolls.
    REQUIRE(fixture.Frame->View()->SingleAxisScroll(ScrollEventAxis::Vertical, 100));
    CHECK(fixture.Frame->View()->MaintainedScrollPositionAnchor() == nullptr);
    (void)fixture.ById(u8"first").SetAttribute(u8"style", u8"height: 1200px");
    (void)fixture.Document->UpdateLayoutIgnorePendingStylesheets();
    html::EventLoop::Current().RunUntilIdle();
    CHECK(fixture.TopOf(u8"section") != 0);
  }

  TEST_CASE("A document's own fragment is scrolled to once it's parsed", "[Loader][FragmentNavigation]")
  {
    auto page = Page::Create();
    auto &frame = page->MainFrame();
    frame.CreateView({400, 300});
    auto document = CreateRef<dom::HTMLDocument>();
    document->SetURL(url::URL(u8"https://example.com/page.html#section"));
    frame.SetDocument(ShareRefPtr<dom::Document>(document.get()));

    auto parser = CreateRef<html::HTMLDocumentParser>(*document);
    parser->InputStream().Append(u8"<!DOCTYPE html>" + utf8_string(Page), IsEOF(true));
    (void)parser->PumpTokenizer();
    html::EventLoop::Current().RunUntilIdle();

    auto section = document->GetElementById(u8"section");
    REQUIRE(section);
    CHECK(document->CSSTarget() == section.get());
    (void)document->UpdateLayoutIgnorePendingStylesheets();
    CHECK(section->GetBoundingClientRect()->Y() == 0);
    frame.SetDocument(nullptr);
  }

  TEST_CASE("Text fragments", "[Loader][FragmentNavigation]")
  {
    FragmentFixture fixture;
    using namespace std::chrono_literals;

    SECTION("a text directive highlights its text, makes its element the target and scrolls it into view")
    {
      fixture.Navigate(u8"#:~:text=Caf%C3%A9");
      // The directive is taken from the URL, and with it the fragment, which it was all of.
      CHECK(fixture.DocumentURL() == "https://example.com/dir/page.html");
      CHECK(fixture.Document->FragmentDirective() == u8"text=Caf%C3%A9");
      CHECK(fixture.Target() == &fixture.ById(u8"café"));
      REQUIRE(fixture.Document->FragmentHighlightRegistryIfExists());
      CHECK_FALSE(fixture.Document->FragmentHighlightRegistryIfExists()->IsEmpty());
      // Centred in the view.
      auto top = fixture.TopOf(u8"café");
      CHECK(top > 0);
      CHECK(top < 300);
      // The selection isn't changed.
      CHECK(fixture.Document->Selection().IsNone());

      // A fragment without a directive goes to its element, and the text isn't highlighted any more.
      fixture.Navigate(u8"#section");
      CHECK(fixture.Target() == &fixture.ById(u8"section"));
      CHECK(fixture.Document->FragmentDirective().empty());
      CHECK(fixture.Document->FragmentHighlightRegistryIfExists()->IsEmpty());
    }

    SECTION("text that isn't there yet is tried again while the page loads")
    {
      fixture.Navigate(u8"#:~:text=late%20arrival");
      CHECK(fixture.Target() == nullptr);

      auto paragraph = fixture.Document->CreateElement(u8"p").ReleaseValue();
      (void)paragraph->SetAttribute(u8"id", u8"late");
      (void)paragraph->TextContent(dom::DOMString(u8"A late arrival"));
      REQUIRE_FALSE(fixture.ById(u8"section").ParentNode()->AppendChild(*paragraph).HasException());
      (void)fixture.Document->UpdateLayoutIgnorePendingStylesheets();

      auto &loop = html::EventLoop::Current();
      loop.Run(loop.Now() + 600ms);
      CHECK(fixture.Target() == &fixture.ById(u8"late"));
    }

    SECTION("the retries stop after three seconds")
    {
      fixture.Navigate(u8"#:~:text=never");
      auto &loop = html::EventLoop::Current();
      for (int32 step = 0; step < 8; ++step)
      {
        loop.Run(loop.Now() + 500ms);
      }
      auto paragraph = fixture.Document->CreateElement(u8"p").ReleaseValue();
      (void)paragraph->TextContent(dom::DOMString(u8"never"));
      REQUIRE_FALSE(fixture.ById(u8"section").ParentNode()->AppendChild(*paragraph).HasException());
      (void)fixture.Document->UpdateLayoutIgnorePendingStylesheets();
      loop.Run(loop.Now() + 600ms);
      CHECK(fixture.Target() == nullptr);
    }
  }
}

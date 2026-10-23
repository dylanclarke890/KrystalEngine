#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/URL/URL.hpp"

namespace krys::boo::tests
{
  namespace
  {
    void Navigate(NavigationFixture &fixture, utf8_stringview url,
                  dom::DOMStringAtom target = dom::DOMStringAtom::Empty()) noexcept
    {
      fixture.Frame->Loader().ChangeLocation(url::URL(fixture.Document->GetURL(), url), target, nullptr,
                                             ReferrerPolicy::EmptyString, std::nullopt,
                                             dom::DOMStringAtom::Null());
    }
  }

  TEST_CASE("FrameLoader: navigating within the document", "[Loader][FrameLoader]")
  {
    NavigationFixture fixture(u8"<p id=p>Text</p>");

    SECTION("an allowed fragment navigation changes the document's URL")
    {
      Navigate(fixture, u8"#p");
      REQUIRE(fixture.Navigations().size() == 1);
      CHECK(fixture.Navigations()[0].Action.Type() == NavigationType::Other);
      CHECK(fixture.DocumentURL() == "https://example.com/dir/page.html#p");
      CHECK(fixture.Client->DidNavigateWithinPageCount == 1);
      CHECK(fixture.Client->DidChangeLocationWithinPageCount == 1);

      // The same fragment again navigates, but doesn't change the location.
      Navigate(fixture, u8"#p");
      CHECK(fixture.Client->DidNavigateWithinPageCount == 2);
      CHECK(fixture.Client->DidChangeLocationWithinPageCount == 1);

      // An empty fragment is a fragment too.
      Navigate(fixture, u8"#");
      CHECK(fixture.DocumentURL() == "https://example.com/dir/page.html#");
      CHECK(fixture.Client->DidChangeLocationWithinPageCount == 2);
    }

    SECTION("an ignored one changes nothing")
    {
      fixture.Client->Policy = PolicyAction::Ignore;
      Navigate(fixture, u8"#p");
      CHECK(fixture.Navigations().size() == 1);
      CHECK(fixture.DocumentURL() == "https://example.com/dir/page.html");
      CHECK(fixture.Client->DidNavigateWithinPageCount == 0);
    }

    SECTION("the same URL without a fragment, or another URL, loads a document")
    {
      Navigate(fixture, u8"page.html");
      Navigate(fixture, u8"other.html#p");
      CHECK(fixture.Navigations().size() == 2);
      CHECK(fixture.DocumentURL() == "https://example.com/dir/page.html");
      CHECK(fixture.Client->DidNavigateWithinPageCount == 0);
    }

    SECTION("targets")
    {
      Navigate(fixture, u8"#a", dom::DOMStringAtom(u8"_self"));
      Navigate(fixture, u8"#b", dom::DOMStringAtom(u8"_TOP"));
      Navigate(fixture, u8"#c", dom::DOMStringAtom(u8"_parent"));
      CHECK(fixture.DocumentURL() == "https://example.com/dir/page.html#c");
      CHECK(fixture.Client->DidNavigateWithinPageCount == 3);

      Navigate(fixture, u8"#d", dom::DOMStringAtom(u8"_blank"));
      Navigate(fixture, u8"#e", dom::DOMStringAtom(u8"elsewhere"));
      REQUIRE(fixture.Navigations().size() == 5);
      CHECK(fixture.Navigations()[3].IsNewWindow);
      CHECK(fixture.Navigations()[4].NewWindowName == "elsewhere");
      CHECK(fixture.DocumentURL() == "https://example.com/dir/page.html#c");
    }
  }

  TEST_CASE("FrameLoader::ShouldReload", "[Loader][FrameLoader]")
  {
    url::URL page(u8"https://example.com/a?q#x");
    CHECK(FrameLoader::ShouldReload(page, url::URL(u8"https://example.com/a?q")));
    CHECK_FALSE(FrameLoader::ShouldReload(page, url::URL(u8"https://example.com/a?q#y")));
    CHECK_FALSE(FrameLoader::ShouldReload(page, url::URL(u8"https://example.com/a?q#")));
    CHECK(FrameLoader::ShouldReload(page, url::URL(u8"https://example.com/a?r#x")));
  }

  TEST_CASE("FrameLoader: form submissions", "[Loader][FrameLoader]")
  {
    NavigationFixture fixture(u8R"(<form id=get action="#done" target=_self><input name=n value=v></form>
      <form id=post action="submit" method=post target=results><input name=n value=v></form>)");
    fixture.Client->Policy = PolicyAction::Ignore;

    fixture.Get<html::HTMLFormElement>(u8"get").Submit();
    REQUIRE(fixture.Navigations().size() == 1);
    auto &get = fixture.Navigations()[0];
    CHECK(get.URL == "https://example.com/dir/page.html?n=v#done");
    CHECK(get.HTTPMethod == "GET");
    CHECK(get.Action.Type() == NavigationType::FormSubmitted);
    CHECK(get.Action.SourceElement() == &fixture.ById(u8"get"));
    REQUIRE(get.Submission);
    // A target naming the frame itself is cleared.
    CHECK(Str(get.Submission->Target()).empty());
    CHECK_FALSE(get.IsNewWindow);

    fixture.Get<html::HTMLFormElement>(u8"post").Submit();
    REQUIRE(fixture.Navigations().size() == 2);
    auto &post = fixture.Navigations()[1];
    CHECK(post.URL == "https://example.com/dir/submit");
    CHECK(post.HTTPMethod == "POST");
    CHECK(post.IsNewWindow);
    CHECK(post.NewWindowName == "results");
  }

  TEST_CASE("FrameLoader: a page without a host ignores navigations", "[Loader][FrameLoader]")
  {
    auto frame = LocalFrame::Create();
    CHECK(frame->Loader().Client().DispatchDecidePolicyForNavigationAction(NavigationAction(),
                                                                           ResourceRequest(), nullptr)
          == PolicyAction::Ignore);
  }
}

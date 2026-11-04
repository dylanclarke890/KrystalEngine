#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
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
                                             ReferrerPolicy::EmptyString, dom::DOMStringAtom::Null());
    }
  }

  TEST_CASE("FrameLoader: navigating within the document", "[Loader][FrameLoader]")
  {
    NavigationFixture fixture(u8"<p id=p>Text</p>");

    SECTION("a fragment navigation changes the document's URL, without asking the host")
    {
      Navigate(fixture, u8"#p");
      CHECK(fixture.Navigations().empty());
      CHECK(fixture.DocumentURL() == "https://example.com/dir/page.html#p");
      CHECK(fixture.Client->DidNavigateWithinPageCount == 1);

      // The same fragment again navigates too.
      Navigate(fixture, u8"#p");
      CHECK(fixture.Client->DidNavigateWithinPageCount == 2);

      // An empty fragment is a fragment too.
      Navigate(fixture, u8"#");
      CHECK(fixture.DocumentURL() == "https://example.com/dir/page.html#");
    }

    SECTION("the same URL without a fragment, or another URL, goes to the host, and the page stays")
    {
      Navigate(fixture, u8"page.html");
      Navigate(fixture, u8"other.html#p");
      REQUIRE(fixture.Navigations().size() == 2);
      CHECK(fixture.Navigations()[0].URL == "https://example.com/dir/page.html");
      CHECK(fixture.Navigations()[1].Action.Type() == NavigationType::Other);
      CHECK_FALSE(fixture.Navigations()[1].IsNewWindow);
      CHECK(fixture.DocumentURL() == "https://example.com/dir/page.html");
      CHECK(fixture.Client->DidNavigateWithinPageCount == 0);
      CHECK_FALSE(fixture.Frame->Loader().IsLoading());
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
      REQUIRE(fixture.Navigations().size() == 2);
      CHECK(fixture.Navigations()[0].IsNewWindow);
      CHECK(fixture.Navigations()[1].NewWindowName == "elsewhere");
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

    // A submission reaches the host on a later task, even to a fragment of the document.
    fixture.Get<html::HTMLFormElement>(u8"get").Submit();
    CHECK(fixture.Client->Navigations.empty());
    REQUIRE(fixture.Navigations().size() == 1);
    auto &get = fixture.Navigations()[0];
    CHECK(get.URL == "https://example.com/dir/page.html#done");
    CHECK(get.HTTPMethod == "GET");
    CHECK(get.Action.Type() == NavigationType::FormSubmitted);
    CHECK(get.Action.SourceElement() == &fixture.ById(u8"get"));
    REQUIRE(get.Submission);
    // A target naming the frame itself is cleared.
    CHECK(Str(get.Submission->Target()).empty());
    CHECK_FALSE(get.IsNewWindow);
    CHECK(fixture.DocumentURL() == "https://example.com/dir/page.html");

    fixture.Get<html::HTMLFormElement>(u8"post").Submit();
    REQUIRE(fixture.Navigations().size() == 2);
    auto &post = fixture.Navigations()[1];
    CHECK(post.URL == "https://example.com/dir/submit");
    CHECK(post.HTTPMethod == "POST");
    CHECK(post.IsNewWindow);
    CHECK(post.NewWindowName == "results");
  }

  TEST_CASE("FrameLoader: a later submission of a form cancels the planned one", "[Loader][FrameLoader]")
  {
    NavigationFixture fixture(u8R"(<form id=form action="first"><input name=n value=v></form>)");
    auto &form = fixture.Get<html::HTMLFormElement>(u8"form");
    (void)form.RequestSubmit();
    form.Action(u8"second");
    (void)form.RequestSubmit();
    REQUIRE(fixture.Navigations().size() == 1);
    CHECK(fixture.Navigations()[0].URL == "https://example.com/dir/second");
  }
}

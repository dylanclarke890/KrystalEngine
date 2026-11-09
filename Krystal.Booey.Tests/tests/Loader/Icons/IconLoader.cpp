#include "Krystal.Booey/Loader/Icons/IconLoader.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey/DOM/Collections/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHeadElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLLinkElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Loader/DocumentLoader.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief Loads @p markup at an `https:` URL and runs the tasks that finish it.
    void LoadPage(LoadFixture &fixture, utf8_string &&markup) noexcept
    {
      fixture.LoadMarkup(krys::move(markup), u8"https://example.com/page.html");
      html::EventLoop::Current().RunUntilIdle();
      RunNetworkingTasks();
    }

    KRYS_NODISCARD List<std::string> IconURLs(const List<Pair<LinkIcon, uint64>> &icons) noexcept
    {
      List<std::string> urls;
      for (const auto &[icon, identifier] : icons)
      {
        urls.push_back(Narrow(icon.URL.String()));
      }
      return urls;
    }
  }

  TEST_CASE("Icons: once a document loads, the client decides on its icons, the largest first",
            "[Loader][Icons][html:rel-icon]")
  {
    LoadFixture fixture;
    LoadPage(fixture, u8"<!DOCTYPE html><link rel=stylesheet href=a.css>"
                      u8"<link rel=icon href=small.png sizes=16x16 type=image/png>"
                      u8"<link rel=\"shortcut icon\" href=large.png sizes=\"32x32 64x64\">"
                      u8"<link rel=icon href=unsized.ico><link rel=icon href=ftp://example.com/x.ico>"
                      u8"<link rel=icon href=''>");

    REQUIRE(fixture.Client->IconDecisions.size() == 1);
    auto &icons = fixture.Client->IconDecisions[0];
    CHECK(IconURLs(icons)
          == List<std::string> {"https://example.com/large.png", "https://example.com/small.png",
                                "https://example.com/unsized.ico"});
    CHECK(icons[0].first.Size == 32u);
    CHECK(icons[1].first.MIMEType == u8"image/png");
    CHECK_FALSE(icons[2].first.Size);
    CHECK(fixture.Frame().Loader().GetDocumentLoader()->LinkIcons().size() == 3);
  }

  TEST_CASE("Icons: an icon the client decides to load is handed to it", "[Loader][Icons][html:rel-icon]")
  {
    LoadFixture fixture;
    fixture.Strategy.AddText(u8"https://example.com/icon.png", u8"PNG bytes", u8"image/png");
    // An error page isn't an icon.
    fixture.Strategy.AddText(u8"https://example.com/error.png", u8"Not found", u8"text/html");
    fixture.Strategy.StatusCodes[u8"https://example.com/error.png"] = 404;
    LoadPage(fixture, u8"<!DOCTYPE html><link rel=icon href=icon.png><link rel=icon href=missing.png>"
                      u8"<link rel=icon href=error.png>");
    REQUIRE(fixture.Client->IconDecisions.size() == 1);
    auto icons = fixture.Client->IconDecisions[0];
    REQUIRE(icons.size() == 3);

    auto &documentLoader = *fixture.Frame().Loader().GetDocumentLoader();
    List<std::string> loaded;
    for (const auto &[icon, identifier] : icons)
    {
      documentLoader.DidGetLoadDecisionForIcon(
        true, identifier, [&](const SharedBuffer *data)
        { loaded.push_back(data ? std::string(data->GetSpan().begin(), data->GetSpan().end()) : "none"); });
    }
    RunNetworkingTasks();
    CHECK(loaded == List<std::string> {"PNG bytes", "none", "none"});

    SECTION("an icon declined, or decided on twice, isn't loaded")
    {
      bool called = false;
      documentLoader.DidGetLoadDecisionForIcon(true, icons[0].second,
                                               [&](const SharedBuffer *data) { called = !data; });
      CHECK(called);
    }
  }

  TEST_CASE("Icons: without icon links, an http(s) document's icon is /favicon.ico",
            "[Loader][Icons][html:rel-icon]")
  {
    LoadFixture fixture;
    LoadPage(fixture, u8"<!DOCTYPE html><link rel=stylesheet href=a.css>");
    REQUIRE(fixture.Client->IconDecisions.size() == 1);
    CHECK(IconURLs(fixture.Client->IconDecisions[0])
          == List<std::string> {"https://example.com/favicon.ico"});
  }

  TEST_CASE("Icons: the client is asked again as the icons change", "[Loader][Icons][html:rel-icon]")
  {
    LoadFixture fixture;
    LoadPage(fixture, u8"<!DOCTYPE html><link id=l rel=icon href=a.png>");
    REQUIRE(fixture.Client->IconDecisions.size() == 1);
    auto &link = Downcast<html::HTMLLinkElement>(*fixture.Document().GetElementById(u8"l"));

    // The same icons aren't decided on again.
    (void)link.SetAttribute(u8"href", u8"a.png");
    CHECK(fixture.Client->IconDecisions.size() == 1);

    (void)link.SetAttribute(u8"href", u8"b.png");
    REQUIRE(fixture.Client->IconDecisions.size() == 2);
    CHECK(IconURLs(fixture.Client->IconDecisions[1]) == List<std::string> {"https://example.com/b.png"});

    auto added = fixture.Document().CreateElement(u8"link").ReleaseValue();
    (void)added->SetAttribute(u8"rel", u8"icon");
    (void)added->SetAttribute(u8"href", u8"c.png");
    (void)added->SetAttribute(u8"sizes", u8"48x48");
    (void)fixture.Document().Head()->AppendChild(*added);
    REQUIRE(fixture.Client->IconDecisions.size() == 3);
    CHECK(IconURLs(fixture.Client->IconDecisions[2])
          == List<std::string> {"https://example.com/c.png", "https://example.com/b.png"});

    // Without icon links, it's the default again.
    (void)link.SetAttribute(u8"rel", u8"stylesheet");
    added->Remove();
    REQUIRE(fixture.Client->IconDecisions.size() == 5);
    CHECK(IconURLs(fixture.Client->IconDecisions[4])
          == List<std::string> {"https://example.com/favicon.ico"});
  }

  TEST_CASE("Icons: aren't loaded with icon loading off, or for about: documents",
            "[Loader][Icons][html:rel-icon]")
  {
    LoadFixture fixture;
    SECTION("off")
    {
      fixture.Document().GetSettings().IconLoadingEnabled = false;
      LoadPage(fixture, u8"<!DOCTYPE html><link rel=icon href=a.png>");
      fixture.Document().GetSettings().IconLoadingEnabled = true;
    }
    SECTION("about:blank")
    {
      fixture.LoadMarkup(u8"<!DOCTYPE html><link rel=icon href=https://example.com/a.png>", u8"about:blank");
      html::EventLoop::Current().RunUntilIdle();
      RunNetworkingTasks();
    }
    CHECK(fixture.Client->IconDecisions.empty());
  }

  TEST_CASE("Icons: rel supports icon", "[Loader][Icons][html:rel-icon]")
  {
    LoadFixture fixture;
    auto link = fixture.Document().CreateElement(u8"link").ReleaseValue();
    CHECK(Downcast<html::HTMLLinkElement>(*link).RelList().Supports(u8"icon").ReleaseValue());
  }
}

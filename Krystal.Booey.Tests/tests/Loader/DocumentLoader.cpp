#include "Krystal.Booey/Loader/DocumentLoader.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/BeforeUnloadEvent.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/PageTransitionEvent.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/DocumentWriter.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/Loader/LocalFrameLoaderClient.hpp"
#include "Krystal.Booey/Loader/NavigationAction.hpp"
#include "Krystal.Booey/Loader/NavigationScheduler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Location.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

namespace krys::boo::tests
{
  TEST_CASE("A frame starts with its initial empty document", "[Loader][DocumentLoader]")
  {
    LoadFixture fixture;
    auto &loader = fixture.Frame().Loader();

    CHECK(loader.IsDisplayingInitialEmptyDocument());
    CHECK(fixture.Document().GetURL().String() == u8"about:blank");
    CHECK(fixture.Document().Window() != nullptr);
    CHECK(fixture.Document().DocumentElement() != nullptr);
    CHECK_FALSE(loader.IsLoading());
    REQUIRE(loader.GetDocumentLoader());
    CHECK(loader.GetDocumentLoader()->IsCommitted());
  }

  TEST_CASE("A document loads on a later task, and commits", "[Loader][DocumentLoader]")
  {
    LoadFixture fixture;
    auto &loader = fixture.Frame().Loader();
    RefPtr<dom::Document> initial = ShareRefPtr(&fixture.Document());
    auto *initialWindow = initial->Window();

    fixture.LoadMarkup(u8"<!DOCTYPE html><p id=text>Hello</p>", u8"https://example.com/page.html");

    // Provisionally: the frame still shows its old document.
    CHECK(loader.IsLoading());
    CHECK(loader.State() == FrameState::Provisional);
    CHECK(loader.ProvisionalDocumentLoader() != nullptr);
    CHECK(&fixture.Document() == initial.get());
    CHECK(fixture.Events == List<std::string> {"policy (host)", "start"});

    html::EventLoop::Current().RunUntilIdle();
    CHECK_FALSE(loader.IsLoading());
    CHECK(&fixture.Document() != initial.get());
    CHECK(fixture.Document().GetURL().String() == u8"https://example.com/page.html");
    CHECK(fixture.Document().GetElementById(u8"text"));
    CHECK(fixture.Document().Window() != initialWindow);
    CHECK_FALSE(loader.IsDisplayingInitialEmptyDocument());
    CHECK(fixture.Events
          == List<std::string> {"policy (host)", "start", "window", "commit", "document loaded", "finish"});
  }

  TEST_CASE("A loaded document attaches declarative shadow roots", "[Loader][DocumentLoader]")
  {
    LoadFixture fixture;
    fixture.LoadMarkup(
      u8"<!DOCTYPE html><div id=host><template shadowrootmode=open><b>s</b></template></div>",
      u8"https://example.com/page.html");
    html::EventLoop::Current().RunUntilIdle();

    auto host = fixture.Document().GetElementById(u8"host");
    REQUIRE(host);
    CHECK_FALSE(host->HasChildNodes());
    auto shadow = host->ShadowRoot();
    REQUIRE(shadow);
    CHECK(shadow->InnerHTML() == u8"<b>s</b>");
  }

  TEST_CASE("Documents load from the loader strategy and data: URLs", "[Loader][DocumentLoader]")
  {
    LoadFixture fixture;
    fixture.Strategy.AddText(u8"https://example.com/served.html", u8"<!DOCTYPE html><p id=served>Served</p>",
                             u8"text/html");

    SECTION("the strategy's")
    {
      fixture.LoadURL(u8"https://example.com/served.html");
      html::EventLoop::Current().RunUntilIdle();
      CHECK(fixture.Document().GetElementById(u8"served"));
      CHECK(fixture.Document().GetURL().String() == u8"https://example.com/served.html");
    }

    SECTION("a data: URL's")
    {
      fixture.LoadURL(u8"data:text/html,<p id=data>Data</p>");
      html::EventLoop::Current().RunUntilIdle();
      CHECK(fixture.Document().GetElementById(u8"data"));
    }

    SECTION("a failure keeps the old document")
    {
      RefPtr<dom::Document> before = ShareRefPtr(&fixture.Document());
      fixture.LoadURL(u8"https://example.com/missing.html");
      html::EventLoop::Current().RunUntilIdle();
      CHECK(&fixture.Document() == before.get());
      CHECK_FALSE(fixture.Frame().Loader().IsLoading());
      CHECK(fixture.Events == List<std::string> {"policy (host)", "start", "fail provisional"});
    }

    SECTION("an ignored navigation loads nothing")
    {
      fixture.Client->Policy = PolicyAction::Ignore;
      fixture.LoadURL(u8"https://example.com/served.html");
      CHECK_FALSE(fixture.Frame().Loader().IsLoading());
      CHECK(fixture.Events == List<std::string> {"policy (host)"});
    }
  }

  TEST_CASE("Stopping and replacing provisional loads", "[Loader][DocumentLoader]")
  {
    LoadFixture fixture;
    fixture.Strategy.Defer = true;
    fixture.Strategy.AddText(u8"https://example.com/a.html", u8"<p id=a>", u8"text/html");
    fixture.Strategy.AddText(u8"https://example.com/b.html", u8"<p id=b>", u8"text/html");
    RefPtr<dom::Document> before = ShareRefPtr(&fixture.Document());

    SECTION("a stopped load's data is dropped")
    {
      fixture.LoadURL(u8"https://example.com/a.html");
      fixture.Frame().Loader().StopAllLoaders();
      CHECK_FALSE(fixture.Frame().Loader().IsLoading());
      fixture.Strategy.CompleteDeferred();
      html::EventLoop::Current().RunUntilIdle();
      CHECK(&fixture.Document() == before.get());
      CHECK(fixture.Events == List<std::string> {"policy (host)", "start", "fail provisional (cancelled)"});
    }

    SECTION("a new load replaces a provisional one")
    {
      fixture.LoadURL(u8"https://example.com/a.html");
      fixture.LoadURL(u8"https://example.com/b.html");
      fixture.Strategy.CompleteDeferred();
      html::EventLoop::Current().RunUntilIdle();
      CHECK(fixture.Document().GetElementById(u8"b"));
      CHECK_FALSE(fixture.Document().GetElementById(u8"a"));
    }
  }

  TEST_CASE("A link to another document loads it", "[Loader][DocumentLoader]")
  {
    LoadFixture fixture;
    fixture.Strategy.AddText(u8"https://example.com/next.html", u8"<!DOCTYPE html><p id=next>Next</p>",
                             u8"text/html");
    fixture.LoadMarkup(u8"<!DOCTYPE html><a id=link href=next.html>Next</a>",
                       u8"https://example.com/page.html");
    html::EventLoop::Current().RunUntilIdle();
    fixture.Events.clear();

    auto link = fixture.Document().GetElementById(u8"link");
    REQUIRE(link);
    link->DispatchSimulatedClick(nullptr);
    html::EventLoop::Current().RunUntilIdle();
    CHECK(fixture.Document().GetElementById(u8"next"));
    CHECK(fixture.Events.front() == "policy");
  }

  TEST_CASE("The document writer decodes UTF-8 across chunks", "[Loader][DocumentLoader]")
  {
    LoadFixture fixture;
    DocumentWriter writer;
    writer.SetFrame(fixture.Frame());
    REQUIRE(writer.Begin(url::URL(u8"about:blank")));

    // A byte order mark, then é split between two chunks, then an invalid byte.
    const uint8 first[] = {0xEF, 0xBB, 0xBF, '<', 'p', ' ', 'i', 'd', '=', 't', '>', 0xC3};
    const uint8 second[] = {0xA9, 0xFF};
    writer.AddData(Span<const uint8>(first));
    writer.AddData(Span<const uint8>(second));
    writer.End();

    auto paragraph = fixture.Document().GetElementById(u8"t");
    REQUIRE(paragraph);
    CHECK(Narrow(paragraph->TextContent().value_or(u8"")) == "\xC3\xA9\xEF\xBF\xBD");
  }

  namespace
  {
    /// @brief Records the end of loading's events, on each new document and its window.
    struct EndRecorder
    {
      List<std::string> Events;
      List<dom::EventListenerHandle> Handles;

      void Attach(LoadFixture &fixture) noexcept
      {
        fixture.Client->OnClearWindowObject = [this, &fixture]
        {
          Handles.clear();
          auto &document = fixture.Document();
          auto *window = document.Window();
          Handles.push_back(document.AddEventListener(u8"readystatechange",
                                                      [this, &document](dom::Event &)
                                                      {
                                                        switch (document.ReadyState())
                                                        {
                                                          case html::DocumentReadyState::Loading:
                                                            Events.push_back("loading");
                                                            break;
                                                          case html::DocumentReadyState::Interactive:
                                                            Events.push_back("interactive");
                                                            break;
                                                          case html::DocumentReadyState::Complete:
                                                            Events.push_back("complete");
                                                            break;
                                                        }
                                                      }));
          Handles.push_back(document.AddEventListener(u8"DOMContentLoaded",
                                                      [this](dom::Event &event)
                                                      {
                                                        CHECK(event.Bubbles());
                                                        Events.push_back("DOMContentLoaded");
                                                      }));
          Handles.push_back(window->AddEventListener(u8"load",
                                                     [this, &document, window](dom::Event &event)
                                                     {
                                                       CHECK(event.Target().get() == &document);
                                                       CHECK(event.CurrentTarget().get() == window);
                                                       Events.push_back("load");
                                                     }));
          Handles.push_back(
            window->AddEventListener<html::PageTransitionEvent>(u8"pageshow",
                                                                [this](html::PageTransitionEvent &event)
                                                                {
                                                                  CHECK_FALSE(event.Persisted());
                                                                  Events.push_back("pageshow");
                                                                }));
          // load doesn't reach the window from the document's own listeners, and DOMContentLoaded does.
          Handles.push_back(window->AddEventListener(u8"DOMContentLoaded", [this](dom::Event &)
                                                     { Events.push_back("window heard"); }));
        };
      }
    };
  }

  TEST_CASE("The end of parsing and the load event", "[Loader][DocumentLoader]")
  {
    LoadFixture fixture;
    EndRecorder recorder;
    recorder.Attach(fixture);

    SECTION("in order, with the readiness and the timing")
    {
      fixture.LoadMarkup(u8"<!DOCTYPE html><p>Hi</p>", u8"https://example.com/page.html");
      html::EventLoop::Current().RunUntilIdle();
      CHECK(recorder.Events
            == List<std::string> {"loading", "interactive", "DOMContentLoaded", "window heard", "complete",
                                  "load", "pageshow"});
      CHECK(fixture.Document().ReadyState() == html::DocumentReadyState::Complete);
      CHECK(fixture.Document().LoadEventFinished());
      CHECK(fixture.Document().PageShowing());
      auto &timing = fixture.Document().LoadTiming();
      CHECK(timing.DOMContentLoadedEventStartTime <= timing.DOMContentLoadedEventEndTime);
      CHECK(timing.LoadEventStartTime <= timing.LoadEventEndTime);
      CHECK(fixture.Events.back() == "finish");
    }

    SECTION("the load event waits for images and style sheets")
    {
      fixture.Strategy.Defer = true;
      fixture.Strategy.AddText(u8"https://example.com/style.css", u8"p { color: green }");
      fixture.Strategy.AddFile(u8"https://example.com/missing.png", __FILE__, u8"image/png");
      fixture.LoadMarkup(
        u8"<!DOCTYPE html><link rel=stylesheet href=style.css><img src=picture.png><p>Hi</p>",
        u8"https://example.com/page.html");

      // The document arrives; its subresources don't yet.
      while (fixture.Strategy.Deferred.empty() || fixture.Frame().Loader().ProvisionalDocumentLoader())
      {
        fixture.Strategy.CompleteDeferred();
        if (!html::EventLoop::Current().RunNextTask())
        {
          break;
        }
      }
      html::EventLoop::Current().RunUntilIdle();
      CHECK(fixture.Document().GetCachedResourceLoader().RequestCount() > 0);
      CHECK(std::ranges::find(recorder.Events, "DOMContentLoaded") != recorder.Events.end());
      CHECK(std::ranges::find(recorder.Events, "load") == recorder.Events.end());
      CHECK(fixture.Frame().Loader().IsLoading());

      // Once they're done (the image failing), the document has loaded.
      fixture.Strategy.CompleteDeferred();
      html::EventLoop::Current().RunUntilIdle();
      CHECK(fixture.Document().GetCachedResourceLoader().RequestCount() == 0);
      CHECK(recorder.Events.back() == "pageshow");
      CHECK_FALSE(fixture.Frame().Loader().IsLoading());
    }
  }

  TEST_CASE("Leaving a document: beforeunload, pagehide and unload", "[Loader][DocumentLoader]")
  {
    LoadFixture fixture;
    fixture.LoadMarkup(u8"<!DOCTYPE html><body><p>First</p></body>", u8"https://example.com/first.html");
    html::EventLoop::Current().RunUntilIdle();
    RefPtr<dom::Document> first = ShareRefPtr(&fixture.Document());
    auto *window = first->Window();

    List<std::string> events;
    bool stay = false;
    auto beforeUnload =
      window->AddEventListener<html::BeforeUnloadEvent>(u8"beforeunload",
                                                        [&](html::BeforeUnloadEvent &event)
                                                        {
                                                          CHECK(event.Cancelable());
                                                          CHECK(event.Target().get() == first.get());
                                                          events.push_back("beforeunload");
                                                          if (stay)
                                                          {
                                                            event.PreventDefault();
                                                          }
                                                          // No dialogs while unloading.
                                                          window->Alert(u8"Leaving");
                                                        });
    auto pageHide = window->AddEventListener<html::PageTransitionEvent>(u8"pagehide",
                                                                        [&](html::PageTransitionEvent &event)
                                                                        {
                                                                          CHECK_FALSE(event.Persisted());
                                                                          events.push_back("pagehide");
                                                                        });
    auto unload = window->AddEventListener(u8"unload",
                                           [&](dom::Event &event)
                                           {
                                             CHECK(event.Target().get() == first.get());
                                             events.push_back("unload");
                                             window->Alert(u8"Gone");
                                           });

    SECTION("a document is unloaded as the next one commits")
    {
      fixture.LoadMarkup(u8"<!DOCTYPE html><p>Second</p>", u8"https://example.com/second.html");
      CHECK(events == List<std::string> {"beforeunload"});
      html::EventLoop::Current().RunUntilIdle();
      CHECK(events == List<std::string> {"beforeunload", "pagehide", "unload"});
      CHECK_FALSE(first->PageShowing());
      CHECK(fixture.Chrome->Alerts.empty());
      CHECK(fixture.Chrome->ConfirmPanels == 0);
    }

    SECTION("asking to stay needs the user to have interacted with the page")
    {
      stay = true;
      fixture.LoadMarkup(u8"<!DOCTYPE html><p>Second</p>", u8"https://example.com/second.html");
      html::EventLoop::Current().RunUntilIdle();
      CHECK(fixture.Chrome->ConfirmPanels == 0);
      CHECK(&fixture.Document() != first.get());
    }

    SECTION("the host confirms leaving a page that asks to stay")
    {
      stay = true;
      window->NotifyActivated();
      fixture.Chrome->Leave = false;
      fixture.Events.clear();
      fixture.LoadMarkup(u8"<!DOCTYPE html><p>Second</p>", u8"https://example.com/second.html");
      html::EventLoop::Current().RunUntilIdle();
      CHECK(fixture.Chrome->ConfirmPanels == 1);
      CHECK(&fixture.Document() == first.get());
      // The load never started.
      CHECK(fixture.Events == List<std::string> {"policy (host)"});

      fixture.Chrome->Leave = true;
      fixture.LoadMarkup(u8"<!DOCTYPE html><p>Second</p>", u8"https://example.com/second.html");
      html::EventLoop::Current().RunUntilIdle();
      CHECK(fixture.Chrome->ConfirmPanels == 2);
      CHECK(&fixture.Document() != first.get());
    }

    SECTION("closing the window asks too")
    {
      stay = true;
      window->NotifyActivated();
      fixture.Chrome->Leave = false;
      window->Close();
      CHECK_FALSE(window->Closed());
      fixture.Chrome->Leave = true;
      window->Close();
      CHECK(window->Closed());
    }
  }

  namespace
  {
    using namespace std::chrono_literals;

    /// @brief Runs the event loop with the host's time moved on by @p delta.
    void AdvanceTime(MonotonicTime::duration delta) noexcept
    {
      auto &eventLoop = html::EventLoop::Current();
      eventLoop.Run(eventLoop.Now() + delta);
      eventLoop.RunUntilIdle();
    }
  }

  TEST_CASE("Meta refresh redirects once the document has loaded", "[Loader][DocumentLoader]")
  {
    LoadFixture fixture;
    fixture.Strategy.AddText(u8"https://example.com/next.html", u8"<!DOCTYPE html><p id=next>Next</p>",
                             u8"text/html");

    SECTION("after its delay")
    {
      fixture.LoadMarkup(u8"<!DOCTYPE html><meta http-equiv=refresh content='2; url=next.html'>",
                         u8"https://example.com/page.html");
      html::EventLoop::Current().RunUntilIdle();
      CHECK_FALSE(fixture.Document().GetElementById(u8"next"));
      AdvanceTime(1s);
      CHECK_FALSE(fixture.Document().GetElementById(u8"next"));
      AdvanceTime(1s);
      CHECK(fixture.Document().GetElementById(u8"next"));
    }

    SECTION("only the first refresh counts")
    {
      fixture.LoadMarkup(u8"<!DOCTYPE html><meta http-equiv=Refresh content='0; url=next.html'>"
                         u8"<meta http-equiv=refresh content='0; url=other.html'>",
                         u8"https://example.com/page.html");
      html::EventLoop::Current().RunUntilIdle();
      AdvanceTime(0s);
      CHECK(fixture.Document().GetElementById(u8"next"));
    }

    SECTION("a new document cancels the old one's")
    {
      fixture.LoadMarkup(u8"<!DOCTYPE html><meta http-equiv=refresh content='1; url=next.html'>",
                         u8"https://example.com/page.html");
      html::EventLoop::Current().RunUntilIdle();
      fixture.LoadMarkup(u8"<!DOCTYPE html><p id=stay>Stay</p>", u8"https://example.com/stay.html");
      html::EventLoop::Current().RunUntilIdle();
      AdvanceTime(2s);
      CHECK(fixture.Document().GetElementById(u8"stay"));
    }
  }

  TEST_CASE("The content-language and default-style pragmas", "[Loader][DocumentLoader]")
  {
    LoadFixture fixture;
    fixture.LoadMarkup(
      u8"<!DOCTYPE html><meta http-equiv=content-language content=' fr-CA '>"
      u8"<meta http-equiv=default-style content=Night><p id=p>Bonjour</p><p id=en lang=en>Hi</p>",
      u8"https://example.com/page.html");
    html::EventLoop::Current().RunUntilIdle();

    CHECK(fixture.Document().ContentLanguage() == u8"fr-CA");
    auto paragraph = fixture.Document().GetElementById(u8"p");
    REQUIRE(paragraph);
    CHECK(paragraph->Matches(u8":lang(fr)").Value());
    CHECK_FALSE(fixture.Document().GetElementById(u8"en")->Matches(u8":lang(fr)").Value());

    // A list of languages isn't a language.
    fixture.Document().ProcessMetaHttpEquiv(u8"Content-Language", u8"de, en");
    CHECK(fixture.Document().ContentLanguage() == u8"fr-CA");
    fixture.Document().ProcessMetaHttpEquiv(u8"content-language", u8"de");
    CHECK(fixture.Document().ContentLanguage() == u8"de");
    CHECK(paragraph->Matches(u8":lang(de)").Value());
  }

  TEST_CASE("A window's Location", "[Loader][DocumentLoader]")
  {
    LoadFixture fixture;
    fixture.Strategy.AddText(u8"https://example.com/dir/next.html", u8"<!DOCTYPE html><p id=next>Next</p>",
                             u8"text/html");
    fixture.LoadMarkup(u8"<!DOCTYPE html><p id=here>Here</p>", u8"https://example.com/dir/page.html?q=1#top");
    html::EventLoop::Current().RunUntilIdle();
    auto &window = *fixture.Document().Window();
    auto &location = window.GetLocation();
    CHECK(fixture.Document().GetLocation() == &location);

    CHECK(location.Href() == u8"https://example.com/dir/page.html?q=1#top");
    CHECK(location.ToString() == location.Href());
    CHECK(location.Origin() == u8"https://example.com");
    CHECK(location.Protocol() == u8"https:");
    CHECK(location.Host() == u8"example.com");
    CHECK(location.Pathname() == u8"/dir/page.html");
    CHECK(location.Search() == u8"?q=1");
    CHECK(location.Hash() == u8"#top");
    CHECK(location.AncestorOrigins().empty());

    SECTION("setting the hash navigates within the document, at once")
    {
      RefPtr<dom::Document> before = ShareRefPtr(&fixture.Document());
      location.Hash(u8"here");
      CHECK(fixture.Document().GetURL().String() == u8"https://example.com/dir/page.html?q=1#here");
      CHECK(&fixture.Document() == before.get());
    }

    SECTION("assign() navigates on a later task")
    {
      REQUIRE_FALSE(location.Assign(u8"next.html").HasException());
      CHECK(fixture.Frame().GetNavigationScheduler().LocationChangePending());
      CHECK_FALSE(fixture.Document().GetElementById(u8"next"));
      html::EventLoop::Current().RunUntilIdle();
      CHECK(fixture.Document().GetElementById(u8"next"));
      CHECK(fixture.Document().GetURL().String() == u8"https://example.com/dir/next.html");
    }

    SECTION("an unparsable URL is a SyntaxError")
    {
      CHECK(location.SetHref(u8"https://exa mple.com/").HasException());
      CHECK(location.Replace(u8"https://exa mple.com/").HasException());
    }

    SECTION("reload() loads the document again, from the host's markup")
    {
      RefPtr<dom::Document> before = ShareRefPtr(&fixture.Document());
      location.Reload();
      html::EventLoop::Current().RunUntilIdle();
      CHECK(&fixture.Document() != before.get());
      CHECK(fixture.Document().GetElementById(u8"here"));
    }

    SECTION("a document without a browsing context has no location")
    {
      RefPtr<dom::Document> document = ShareRefPtr(&fixture.Document());
      fixture.Frame().SetDocument(nullptr);
      CHECK(document->GetLocation() == nullptr);
    }
  }
}

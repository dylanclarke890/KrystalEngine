#include "Krystal.Booey/Loader/DocumentLoader.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey/DOM/Events/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/HTML/Events/PageTransitionEvent.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/DocumentWriter.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/Loader/LocalFrameLoaderClient.hpp"
#include "Krystal.Booey/Loader/NavigationAction.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"

namespace krys::boo::tests
{
  TEST_CASE("A frame starts with its initial empty document",
            "[Loader][DocumentLoader][html:creating-a-new-browsing-context]"
            "[html:creating-a-new-top-level-browsing-context][html:creating-a-new-top-level-traversable]"
            "[html:initialize-the-navigable][html:fully-active][html:node-navigable][html:nav-document]")
  {
    LoadFixture fixture;
    auto &loader = fixture.Frame().Loader();

    CHECK(loader.IsDisplayingInitialEmptyDocument());
    CHECK(fixture.Document().GetURL().String() == u8"about:blank");
    CHECK(fixture.Document().Window() != nullptr);
    CHECK(fixture.Document().DocumentElement() != nullptr);
    // An HTML document in quirks mode, with a head and a body, that's completely loaded and fully active.
    CHECK(fixture.Document().ContentType() == u8"text/html");
    CHECK(fixture.Document().InQuirksMode());
    CHECK(fixture.Document().Head());
    CHECK(fixture.Document().Body());
    CHECK(fixture.Document().ReadyState() == html::DocumentReadyState::Complete);
    // It gets no load event.
    html::EventLoop::Current().RunUntilIdle();
    CHECK(fixture.Events.empty());
    CHECK(fixture.Document().IsFullyActive());
    CHECK(fixture.Document().Frame() == &fixture.Frame());
    CHECK_FALSE(loader.IsLoading());
    REQUIRE(loader.GetDocumentLoader());
    CHECK(loader.GetDocumentLoader()->IsCommitted());
  }

  TEST_CASE("A document loaded from about:blank has html, head and body elements",
            "[Loader][DocumentLoader][html:navigate-html][html:populate-with-html/head/body]"
            "[html:initialise-the-document-object]")
  {
    LoadFixture fixture;
    fixture.LoadMarkup(u8"<!DOCTYPE html><p>First</p>", u8"https://example.com/first.html");
    html::EventLoop::Current().RunUntilIdle();
    RefPtr<LocalDOMWindow> firstWindow = ShareRefPtr(fixture.Document().Window());

    fixture.LoadURL(u8"about:blank");
    html::EventLoop::Current().RunUntilIdle();
    CHECK(fixture.Document().GetURL().String() == u8"about:blank");
    CHECK_FALSE(fixture.Frame().Loader().IsDisplayingInitialEmptyDocument());
    REQUIRE(fixture.Document().DocumentElement());
    CHECK(fixture.Document().Head());
    CHECK(fixture.Document().Body());
    CHECK_FALSE(fixture.Document().Body()->HasChildNodes());
    // A new document gets a new window.
    CHECK(fixture.Document().Window() != firstWindow.get());
  }

  TEST_CASE("A text document's content is the text of a pre element",
            "[Loader][DocumentLoader][html:read-text][html:navigate-text]")
  {
    auto mimeType = GENERATE(u8"text/plain", u8"text/css", u8"application/json", u8"application/ld+json",
                             u8"text/javascript");
    LoadFixture fixture;
    fixture.Strategy.AddText(u8"https://example.com/notes", u8"<b>bold</b>\n& more", mimeType);
    fixture.LoadURL(u8"https://example.com/notes");
    html::EventLoop::Current().RunUntilIdle();

    auto body = fixture.Document().Body();
    REQUIRE(body);
    auto *pre = dynamic_cast<html::HTMLElement *>(body->FirstChild());
    REQUIRE(pre);
    CHECK(pre->LocalName() == u8"pre");
    CHECK(pre->TextContent() == u8"<b>bold</b>\n& more");
    CHECK(fixture.Document().ContentType() == dom::DOMString(mimeType));
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
    CHECK(fixture.Events == List<std::string> {"start"});

    html::EventLoop::Current().RunUntilIdle();
    CHECK_FALSE(loader.IsLoading());
    CHECK(&fixture.Document() != initial.get());
    CHECK(fixture.Document().GetURL().String() == u8"https://example.com/page.html");
    CHECK(fixture.Document().GetElementById(u8"text"));
    CHECK(fixture.Document().Window() != initialWindow);
    CHECK_FALSE(loader.IsDisplayingInitialEmptyDocument());
    CHECK(fixture.Events == List<std::string> {"start", "window", "commit", "document loaded", "finish"});
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
      CHECK(fixture.Events == List<std::string> {"start", "fail provisional"});
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
      CHECK(fixture.Events == List<std::string> {"start", "fail provisional (cancelled)"});
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

  TEST_CASE("A link to another document goes to the host, which loads it if it wants",
            "[Loader][DocumentLoader]")
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
    CHECK_FALSE(fixture.Document().GetElementById(u8"next"));
    CHECK(fixture.Events == List<std::string> {"navigation to https://example.com/next.html"});

    fixture.LoadURL(u8"https://example.com/next.html");
    html::EventLoop::Current().RunUntilIdle();
    CHECK(fixture.Document().GetElementById(u8"next"));
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
                                                                  CHECK(event.Bubbles());
                                                                  CHECK(event.Cancelable());
                                                                  Events.push_back("pageshow");
                                                                }));
          // load doesn't reach the window from the document's own listeners, and DOMContentLoaded does.
          Handles.push_back(window->AddEventListener(u8"DOMContentLoaded", [this](dom::Event &)
                                                     { Events.push_back("window heard"); }));
        };
      }
    };
  }

  TEST_CASE(
    "The end of parsing and the load event",
    "[Loader][DocumentLoader][html:the-pagetransitionevent-interface][html:fire-a-page-transition-event]")
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

  TEST_CASE(
    "Leaving a document: pagehide and unload",
    "[Loader][DocumentLoader][html:the-pagetransitionevent-interface][html:dom-pagetransitionevent-persisted]"
    "[html:fire-a-page-transition-event][html:navigate]")
  {
    LoadFixture fixture;
    fixture.LoadMarkup(u8"<!DOCTYPE html><body><p>First</p></body>", u8"https://example.com/first.html");
    html::EventLoop::Current().RunUntilIdle();
    RefPtr<dom::Document> first = ShareRefPtr(&fixture.Document());
    auto *window = first->Window();
    fixture.Events.clear();

    List<std::string> events;
    // Navigating from the events does nothing.
    auto tryToNavigate = [&]
    {
      fixture.Frame().Loader().ChangeLocation(
        url::URL(u8"https://example.com/elsewhere.html"), dom::DOMStringAtom::Empty(), nullptr,
        ReferrerPolicy::EmptyString, std::nullopt, dom::DOMStringAtom::Null());
    };
    auto pageHide = window->AddEventListener<html::PageTransitionEvent>(u8"pagehide",
                                                                        [&](html::PageTransitionEvent &event)
                                                                        {
                                                                          CHECK_FALSE(event.Persisted());
                                                                          CHECK(event.Bubbles());
                                                                          CHECK(event.Cancelable());
                                                                          events.push_back("pagehide");
                                                                          tryToNavigate();
                                                                        });
    auto unload = window->AddEventListener(u8"unload",
                                           [&](dom::Event &event)
                                           {
                                             CHECK(event.Target().get() == first.get());
                                             events.push_back("unload");
                                             // No dialogs while unloading.
                                             window->Alert(u8"Gone");
                                             tryToNavigate();
                                           });

    // A document is unloaded as the next one commits.
    fixture.LoadMarkup(u8"<!DOCTYPE html><p>Second</p>", u8"https://example.com/second.html");
    CHECK(events.empty());
    html::EventLoop::Current().RunUntilIdle();
    CHECK(events == List<std::string> {"pagehide", "unload"});
    CHECK(fixture.Document().GetURL().String() == u8"https://example.com/second.html");
    CHECK_FALSE(first->PageShowing());
    CHECK(fixture.Chrome->Alerts.empty());
    CHECK(std::ranges::find(fixture.Events, "navigation to https://example.com/elsewhere.html")
          == fixture.Events.end());
  }

  TEST_CASE("Meta refresh is ignored", "[Loader][DocumentLoader]")
  {
    LoadFixture fixture;
    fixture.LoadMarkup(u8"<!DOCTYPE html><meta http-equiv=refresh content='0; url=next.html'><p id=here>",
                       u8"https://example.com/page.html");
    html::EventLoop::Current().RunUntilIdle();
    fixture.Events.clear();
    auto &eventLoop = html::EventLoop::Current();
    eventLoop.Run(eventLoop.Now() + std::chrono::seconds(1));
    eventLoop.RunUntilIdle();
    CHECK(fixture.Document().GetElementById(u8"here"));
    CHECK(fixture.Events.empty());
  }

  TEST_CASE(
    "The content-language and default-style pragmas",
    "[Loader][DocumentLoader][html:pragma-set-default-language][html:attr-meta-http-equiv-default-style]")
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

  TEST_CASE(
    "Window.print(): a document still loading prints once it has loaded",
    "[Loader][DocumentLoader][html:dom-print][html:print-when-loaded][html:printing-steps][html:the-end]")
  {
    LoadFixture fixture;
    List<std::string> events;
    List<dom::EventListenerHandle> handles;
    fixture.Client->OnClearWindowObject = [&]
    {
      auto *window = fixture.Document().Window();
      handles.push_back(window->AddEventListener(u8"load", [&](dom::Event &) { events.push_back("load"); }));
      handles.push_back(
        window->AddEventListener(u8"beforeprint", [&](dom::Event &) { events.push_back("beforeprint"); }));
      handles.push_back(
        window->AddEventListener(u8"afterprint", [&](dom::Event &) { events.push_back("afterprint"); }));
      window->Print();
    };

    fixture.LoadMarkup(u8"<!DOCTYPE html><p>Page</p>", u8"https://example.com/page.html");
    html::EventLoop::Current().RunUntilIdle();
    CHECK(events == List<std::string> {"load", "beforeprint", "afterprint"});
    CHECK(fixture.Chrome->PrintRequests == 1);

    SECTION("once it has loaded, it prints at once")
    {
      fixture.Document().Window()->Print();
      CHECK(fixture.Chrome->PrintRequests == 2);
    }
  }

}

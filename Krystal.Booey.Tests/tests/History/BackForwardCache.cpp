#include "Krystal.Booey/History/BackForwardCache.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/History/BackForwardController.hpp"
#include "Krystal.Booey/History/CachedPage.hpp"
#include "Krystal.Booey/History/HistoryItem.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/PageTransitionEvent.hpp"
#include "Krystal.Booey/Loader/HistoryController.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    constexpr utf8_stringview PageA = u8"https://example.com/a.html";
    constexpr utf8_stringview PageB = u8"https://example.com/b.html";
    constexpr utf8_stringview PageC = u8"https://example.com/c.html";

    /// @brief Pages a, b and c, taller than the view, with a back/forward cache of @p maxSize pages.
    struct CacheFixture : LoadFixture
    {
      explicit CacheFixture(uint32 maxSize = 2) noexcept
      {
        BackForwardCache::Singleton().SetMaxSize(maxSize);
        for (auto [url, id] : {Pair {PageA, "a"}, Pair {PageB, "b"}, Pair {PageC, "c"}})
        {
          auto markup = std::string("<!DOCTYPE html><body style='margin: 0'><p id=") + id
                        + " style='height: 1000px'>" + id;
          Texts.push_back(utf8_string(markup.begin(), markup.end()));
          Strategy.AddText(url, Texts.back(), u8"text/html");
        }
      }

      ~CacheFixture() noexcept
      {
        BackForwardCache::Singleton().SetMaxSize(0);
      }

      List<utf8_string> Texts;

      KRYS_NODISCARD BackForwardController &BackForward() const noexcept
      {
        return TestPage->BackForward();
      }

      void Visit(utf8_stringview url) noexcept
      {
        LoadURL(url);
        Run();
      }

      static void Run() noexcept
      {
        html::EventLoop::Current().RunUntilIdle();
      }

      KRYS_NODISCARD bool Shows(utf8_stringview id) const noexcept
      {
        return Document().GetElementById(id) != nullptr;
      }
    };

    /// @brief Records the page transition events at a window, and whether they were persisted.
    struct TransitionRecorder
    {
      List<std::string> Events;
      List<dom::EventListenerHandle> Handles;

      void Attach(LocalDOMWindow &window) noexcept
      {
        for (auto type : {"pagehide", "pageshow", "unload"})
        {
          Handles.push_back(window.AddEventListener(
            dom::DOMStringAtom(utf8_stringview(reinterpret_cast<const char8 *>(type))),
            [this, type = std::string(type)](dom::Event &event)
            {
              auto *transition = DynamicDowncast<html::PageTransitionEvent>(event);
              Events.push_back(type + (transition && transition->Persisted() ? " (persisted)" : ""));
            }));
        }
      }
    };
  }

  TEST_CASE("Going back and forward restores cached pages", "[History][BackForwardCache]")
  {
    CacheFixture fixture;
    auto &cache = BackForwardCache::Singleton();
    fixture.Visit(PageA);
    RefPtr<dom::Document> documentA = ShareRefPtr(&fixture.Document());
    TransitionRecorder recorderA;
    recorderA.Attach(*documentA->Window());

    fixture.Visit(PageB);
    RefPtr<dom::Document> documentB = ShareRefPtr(&fixture.Document());
    TransitionRecorder recorderB;
    recorderB.Attach(*documentB->Window());

    // A was cached as it was left: pagehide, persisted, and no unload.
    CHECK(cache.PageCount() == 1);
    CHECK(fixture.BackForward().BackItem()->IsInBackForwardCache());
    CHECK(recorderA.Events == List<std::string> {"pagehide (persisted)"});
    CHECK(documentA->GetBackForwardCacheState() == dom::BackForwardCacheState::InBackForwardCache);
    CHECK(documentA->IsSuspended());
    CHECK(documentA->Frame() == nullptr);

    // Going back shows it again at once, without loading it.
    fixture.Events.clear();
    REQUIRE(fixture.BackForward().GoBack());
    CHECK(&fixture.Document() == documentA.get());
    CHECK(documentA->Frame() == &fixture.Frame());
    CHECK_FALSE(documentA->IsSuspended());
    CHECK(documentA->GetBackForwardCacheState() == dom::BackForwardCacheState::NotInBackForwardCache);
    CHECK(recorderA.Events == List<std::string> {"pagehide (persisted)", "pageshow (persisted)"});
    CHECK(fixture.Events == List<std::string> {"policy", "start", "commit", "finish"});
    CHECK_FALSE(fixture.Frame().Loader().IsLoading());
    CHECK(fixture.Shows(u8"a"));

    // B went into the cache in its place.
    CHECK(recorderB.Events == List<std::string> {"pagehide (persisted)"});
    CHECK(cache.PageCount() == 1);
    CHECK(fixture.BackForward().ForwardItem()->IsInBackForwardCache());

    REQUIRE(fixture.BackForward().GoForward());
    CHECK(&fixture.Document() == documentB.get());
    CHECK(recorderB.Events == List<std::string> {"pagehide (persisted)", "pageshow (persisted)"});
    CHECK(fixture.BackForward().BackItem()->IsInBackForwardCache());

    SECTION("a new load drops the pages after the current item")
    {
      REQUIRE(fixture.BackForward().GoBack());
      CHECK(fixture.BackForward().ForwardItem()->IsInBackForwardCache());
      fixture.Visit(PageC);
      // B's item went from the list, and its page from the cache; A was cached as it was left.
      CHECK(cache.PageCount() == 1);
      CHECK(fixture.BackForward().BackItem()->IsInBackForwardCache());
      CHECK(documentB->GetBackForwardCacheState() == dom::BackForwardCacheState::NotInBackForwardCache);
      CHECK(documentB->ActiveDOMObjectsAreStopped());
    }
  }

  TEST_CASE("A cached page's timers wait until it's shown again", "[History][BackForwardCache]")
  {
    CacheFixture fixture;
    fixture.Visit(PageA);
    List<std::string> fired;
    (void)fixture.Document().Window()->SetTimeout([&] { fired.push_back("timeout"); }, 50);

    fixture.Visit(PageB);
    auto &eventLoop = html::EventLoop::Current();
    eventLoop.Run(eventLoop.Now() + 200ms);
    CHECK(fired.empty());

    REQUIRE(fixture.BackForward().GoBack());
    eventLoop.Run(eventLoop.Now() + 60ms);
    CHECK(fired == List<std::string> {"timeout"});
  }

  TEST_CASE("A cached page keeps its scroll position", "[History][BackForwardCache]")
  {
    CacheFixture fixture;
    fixture.Visit(PageA);
    fixture.TestPage->UpdateRendering();
    fixture.Frame().View()->SetScrollPosition(IntPoint(0, 300));
    RefPtr<LocalFrameView> viewA = ShareRefPtr(fixture.Frame().View());

    fixture.Visit(PageB);
    CHECK(fixture.Frame().View() != viewA.get());
    CHECK(fixture.BackForward().BackItem()->ScrollPosition() == IntPoint(0, 300));

    REQUIRE(fixture.BackForward().GoBack());
    fixture.TestPage->UpdateRendering();
    CHECK(fixture.Frame().View() == viewA.get());
    CHECK(fixture.Frame().View()->GetScrollPosition().Y() == 300);
  }

  TEST_CASE("Pages that aren't cached", "[History][BackForwardCache]")
  {
    SECTION("without a cache")
    {
      CacheFixture fixture(0);
      fixture.Visit(PageA);
      fixture.Visit(PageB);
      CHECK(BackForwardCache::Singleton().PageCount() == 0);
    }

    SECTION("when the client doesn't allow it")
    {
      CacheFixture fixture;
      fixture.Client->CachePages = false;
      fixture.Visit(PageA);
      RefPtr<dom::Document> documentA = ShareRefPtr(&fixture.Document());
      TransitionRecorder recorder;
      recorder.Attach(*documentA->Window());
      fixture.Visit(PageB);
      CHECK(BackForwardCache::Singleton().PageCount() == 0);
      CHECK(recorder.Events == List<std::string> {"pagehide", "unload"});

      // Going back loads the page again.
      REQUIRE(fixture.BackForward().GoBack());
      fixture.Run();
      CHECK(&fixture.Document() != documentA.get());
      CHECK(fixture.Shows(u8"a"));
    }

    SECTION("on a reload")
    {
      CacheFixture fixture;
      fixture.Visit(PageA);
      fixture.Frame().Loader().Reload();
      fixture.Run();
      CHECK(BackForwardCache::Singleton().PageCount() == 0);
    }

    SECTION("past the cache's size, the oldest goes")
    {
      CacheFixture fixture(1);
      fixture.Visit(PageA);
      fixture.Visit(PageB);
      fixture.Visit(PageC);
      CHECK(BackForwardCache::Singleton().PageCount() == 1);
      CHECK(fixture.BackForward().BackItem()->IsInBackForwardCache());
      CHECK_FALSE(fixture.BackForward().ItemAtIndex(-2)->IsInBackForwardCache());
    }

    SECTION("once it has expired")
    {
      CacheFixture fixture;
      fixture.Visit(PageA);
      RefPtr<dom::Document> documentA = ShareRefPtr(&fixture.Document());
      fixture.Visit(PageB);
      auto &eventLoop = html::EventLoop::Current();
      eventLoop.Run(eventLoop.Now() + CachedPage::ExpirationInterval + 1min);
      CHECK(fixture.BackForward().BackItem()->HasCachedPageExpired());

      REQUIRE(fixture.BackForward().GoBack());
      fixture.Run();
      CHECK(&fixture.Document() != documentA.get());
      CHECK(fixture.Shows(u8"a"));
    }
  }
}

#include "Krystal.Booey/Loader/HistoryController.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/History/BackForwardController.hpp"
#include "Krystal.Booey/History/HistoryItem.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Location.hpp"

namespace krys::boo::tests
{
  namespace
  {
    constexpr utf8_stringview PageA = u8"https://example.com/a.html";
    constexpr utf8_stringview PageB = u8"https://example.com/b.html";
    constexpr utf8_stringview PageC = u8"https://example.com/c.html";

    /// @brief Pages a, b and c, each a paragraph with its letter as its ID, taller than the view.
    struct HistoryFixture : LoadFixture
    {
      HistoryFixture() noexcept
      {
        Strategy.AddText(PageA, u8"<!DOCTYPE html><body style='margin: 0'><p id=a style='height: 1000px'>A",
                         u8"text/html");
        Strategy.AddText(PageB, u8"<!DOCTYPE html><body style='margin: 0'><p id=b style='height: 1000px'>B",
                         u8"text/html");
        Strategy.AddText(PageC, u8"<!DOCTYPE html><body style='margin: 0'><p id=c style='height: 1000px'>C",
                         u8"text/html");
      }

      KRYS_NODISCARD BackForwardController &BackForward() const noexcept
      {
        return TestPage->BackForward();
      }

      KRYS_NODISCARD HistoryController &History() const noexcept
      {
        return Frame().Loader().History();
      }

      /// @brief Loads @p url, and runs the event loop until it has loaded.
      void Visit(utf8_stringview url) noexcept
      {
        LoadURL(url);
        Run();
      }

      static void Run() noexcept
      {
        html::EventLoop::Current().RunUntilIdle();
      }

      KRYS_NODISCARD int32 ScrollY() const noexcept
      {
        return Frame().View()->GetScrollPosition().Y();
      }

      KRYS_NODISCARD bool Shows(utf8_stringview id) const noexcept
      {
        return Document().GetElementById(id) != nullptr;
      }
    };
  }

  TEST_CASE("Loads add history items", "[Loader][HistoryController]")
  {
    HistoryFixture fixture;
    auto &backForward = fixture.BackForward();

    // The initial empty document has no item.
    CHECK_FALSE(backForward.CurrentItem());
    CHECK_FALSE(fixture.History().CurrentItem());

    fixture.Visit(PageA);
    REQUIRE(backForward.CurrentItem());
    CHECK(backForward.CurrentItem()->URL().String() == PageA);
    CHECK(backForward.CurrentItem().get() == fixture.History().CurrentItem());
    CHECK(backForward.BackCount() == 0);

    fixture.Visit(PageB);
    CHECK(backForward.CurrentItem()->URL().String() == PageB);
    REQUIRE(backForward.BackItem());
    CHECK(backForward.BackItem()->URL().String() == PageA);
    CHECK(backForward.BackCount() == 1);
    CHECK(fixture.History().PreviousItem() == backForward.BackItem().get());

    SECTION("loading the current URL again replaces its item")
    {
      RefPtr<HistoryItem> before = backForward.CurrentItem();
      fixture.Visit(PageB);
      CHECK(backForward.BackCount() == 1);
      CHECK(backForward.CurrentItem() == before);
    }

    SECTION("a fragment navigation adds an item for the same document")
    {
      RefPtr<dom::Document> document = ShareRefPtr(&fixture.Document());
      fixture.Document().Window()->GetLocation().Hash(u8"more");
      CHECK(&fixture.Document() == document.get());
      CHECK(backForward.BackCount() == 2);
      CHECK(backForward.CurrentItem()->URL().String() == u8"https://example.com/b.html#more");
      CHECK(backForward.CurrentItem()->DocumentSequenceNumber()
            == backForward.BackItem()->DocumentSequenceNumber());
    }

    SECTION("replace() replaces the current item")
    {
      REQUIRE_FALSE(fixture.Document().Window()->GetLocation().Replace(PageC).HasException());
      fixture.Run();
      CHECK(fixture.Shows(u8"c"));
      CHECK(backForward.BackCount() == 1);
      CHECK(backForward.CurrentItem()->URL().String() == PageC);
      CHECK(backForward.BackItem()->URL().String() == PageA);
    }

    SECTION("assign() adds an item")
    {
      REQUIRE_FALSE(fixture.Document().Window()->GetLocation().Assign(PageC).HasException());
      fixture.Run();
      CHECK(fixture.Shows(u8"c"));
      CHECK(backForward.BackCount() == 2);
    }

    SECTION("a new load drops the items after the current one")
    {
      backForward.GoBack();
      fixture.Run();
      fixture.Visit(PageC);
      CHECK(backForward.ForwardCount() == 0);
      CHECK(backForward.BackCount() == 1);
      CHECK(backForward.BackItem()->URL().String() == PageA);
    }
  }

  TEST_CASE("Going back and forward loads the items' documents", "[Loader][HistoryController]")
  {
    HistoryFixture fixture;
    auto &backForward = fixture.BackForward();
    fixture.Visit(PageA);
    fixture.Visit(PageB);
    RefPtr<HistoryItem> itemA = backForward.BackItem();
    RefPtr<HistoryItem> itemB = backForward.CurrentItem();
    fixture.Events.clear();

    CHECK(backForward.CanGoBackOrForward(-1));
    CHECK_FALSE(backForward.CanGoBackOrForward(1));
    REQUIRE(backForward.GoBack());

    // The cursor moves at once; the document commits on a later task.
    CHECK(backForward.CurrentItem() == itemA);
    CHECK(fixture.Frame().Loader().State() == FrameState::Provisional);
    CHECK(fixture.Frame().Loader().LoadType() == FrameLoadType::Back);
    CHECK(fixture.Shows(u8"b"));

    fixture.Run();
    CHECK(fixture.Shows(u8"a"));
    CHECK(fixture.History().CurrentItem() == itemA.get());
    CHECK(backForward.ForwardCount() == 1);
    CHECK(backForward.BackCount() == 0);
    CHECK(fixture.Events
          == List<std::string> {"policy", "start", "window", "commit", "document loaded", "finish"});

    REQUIRE(backForward.GoForward());
    fixture.Run();
    CHECK(fixture.Shows(u8"b"));
    CHECK(fixture.History().CurrentItem() == itemB.get());
    CHECK_FALSE(backForward.GoForward());

    SECTION("going further than the list goes stops at its end")
    {
      backForward.GoBackOrForward(-5);
      fixture.Run();
      CHECK(fixture.Shows(u8"a"));
    }

    SECTION("an ignored navigation moves the cursor back")
    {
      fixture.Client->Policy = PolicyAction::Ignore;
      REQUIRE(backForward.GoBack());
      CHECK(backForward.CurrentItem() == itemB);
      CHECK(fixture.Shows(u8"b"));
    }

    SECTION("a failed load moves the cursor back")
    {
      fixture.Strategy.Resources.erase(utf8_string(PageA));
      REQUIRE(backForward.GoBack());
      fixture.Frame().Loader().StopAllLoaders();
      CHECK(backForward.CurrentItem() == itemB);
      CHECK(fixture.Shows(u8"b"));
    }

    SECTION("the host's markup loads again")
    {
      fixture.LoadMarkup(u8"<!DOCTYPE html><p id=markup>", u8"https://example.com/markup.html");
      fixture.Run();
      fixture.Visit(PageC);
      REQUIRE(backForward.GoBack());
      fixture.Run();
      CHECK(fixture.Shows(u8"markup"));
      CHECK(fixture.Document().GetURL().String() == u8"https://example.com/markup.html");
    }
  }

  TEST_CASE("Going back to a fragment stays in the document", "[Loader][HistoryController]")
  {
    HistoryFixture fixture;
    auto &backForward = fixture.BackForward();
    fixture.Visit(PageA);
    fixture.Document().Window()->GetLocation().Hash(u8"more");
    fixture.Run();
    RefPtr<dom::Document> document = ShareRefPtr(&fixture.Document());
    REQUIRE(backForward.BackCount() == 1);

    uint32 hashChanges = 0;
    auto listener =
      document->Window()->AddEventListener(u8"hashchange", [&](dom::Event &) { ++hashChanges; });

    REQUIRE(backForward.GoBack());
    CHECK(&fixture.Document() == document.get());
    CHECK(fixture.Document().GetURL().String() == PageA);
    CHECK(fixture.Frame().Loader().State() == FrameState::Complete);
    fixture.Run();
    CHECK(hashChanges == 1);

    REQUIRE(backForward.GoForward());
    CHECK(&fixture.Document() == document.get());
    CHECK(fixture.Document().GetURL().String() == u8"https://example.com/a.html#more");
  }

  TEST_CASE("Scroll positions are saved and restored with history items", "[Loader][HistoryController]")
  {
    HistoryFixture fixture;
    fixture.Visit(PageA);
    fixture.TestPage->UpdateRendering();
    fixture.Frame().View()->SetScrollPosition(IntPoint(0, 300));
    REQUIRE(fixture.ScrollY() == 300);

    // A new document starts at its top.
    fixture.Visit(PageB);
    fixture.TestPage->UpdateRendering();
    CHECK(fixture.ScrollY() == 0);
    CHECK(fixture.BackForward().BackItem()->ScrollPosition() == IntPoint(0, 300));

    SECTION("going back restores it")
    {
      REQUIRE(fixture.BackForward().GoBack());
      fixture.Run();
      fixture.TestPage->UpdateRendering();
      CHECK(fixture.Shows(u8"a"));
      CHECK(fixture.ScrollY() == 300);
    }

    SECTION("unless the item doesn't restore it")
    {
      fixture.BackForward().BackItem()->SetShouldRestoreScrollPosition(false);
      REQUIRE(fixture.BackForward().GoBack());
      fixture.Run();
      fixture.TestPage->UpdateRendering();
      CHECK(fixture.ScrollY() == 0);
    }

    SECTION("a reload keeps the item and its scroll position")
    {
      fixture.Frame().View()->SetScrollPosition(IntPoint(0, 200));
      RefPtr<HistoryItem> item = fixture.BackForward().CurrentItem();
      RefPtr<dom::Document> before = ShareRefPtr(&fixture.Document());
      fixture.Frame().Loader().Reload();
      fixture.Run();
      CHECK(&fixture.Document() != before.get());
      CHECK(fixture.BackForward().CurrentItem() == item);
      CHECK(fixture.BackForward().BackCount() == 1);
      CHECK(fixture.ScrollY() == 200);
    }

    SECTION("location.reload() keeps the item")
    {
      fixture.Document().Window()->GetLocation().Reload();
      fixture.Run();
      CHECK(fixture.Shows(u8"b"));
      CHECK(fixture.BackForward().BackCount() == 1);
      CHECK(fixture.Frame().Loader().LoadType() == FrameLoadType::Reload);
    }
  }

  TEST_CASE("Redirects within a second replace the history item", "[Loader][HistoryController]")
  {
    HistoryFixture fixture;
    fixture.Visit(PageA);
    fixture.Strategy.AddText(u8"https://example.com/refresh.html",
                             u8"<!DOCTYPE html><meta http-equiv=refresh content='0; url=c.html'>",
                             u8"text/html");

    fixture.Visit(u8"https://example.com/refresh.html");
    CHECK(fixture.Shows(u8"c"));
    CHECK(fixture.BackForward().BackCount() == 1);
    CHECK(fixture.BackForward().CurrentItem()->URL().String() == PageC);
    CHECK(fixture.BackForward().BackItem()->URL().String() == PageA);
  }
}

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    /// @brief Runs the event loop with the host's time moved on by @p delta.
    void Advance(MonotonicTime::duration delta) noexcept
    {
      auto &eventLoop = html::EventLoop::Current();
      eventLoop.Run(eventLoop.Now() + delta);
    }

    struct SuspensionFixture : LoadFixture
    {
      SuspensionFixture() noexcept
      {
        LoadMarkup(u8"<!DOCTYPE html><p>Page</p>", u8"https://example.com/page.html");
        html::EventLoop::Current().RunUntilIdle();
      }

      KRYS_NODISCARD LocalDOMWindow &Window() const noexcept
      {
        return *Document().Window();
      }
    };

    constexpr auto Reason = dom::ReasonForSuspension::BackForwardCache;
  }

  TEST_CASE("A suspended document's tasks wait until it resumes", "[DOM][Document][Suspension]")
  {
    SuspensionFixture fixture;
    auto &document = fixture.Document();
    auto &eventLoop = html::EventLoop::Current();
    uint32 fired = 0;
    auto listener = fixture.Window().AddEventListener(u8"custom", [&](dom::Event &) { ++fired; });

    document.Suspend(Reason);
    CHECK(document.IsSuspended());
    CHECK(document.ScheduledTasksAreSuspended());
    CHECK(document.ReasonForSuspendingActiveDOMObjects() == Reason);
    document.QueueTaskToDispatchEventOnWindow(
      html::TaskSource::DOMManipulation, dom::Event::Create(dom::DOMStringAtom(u8"custom"), false, false));
    eventLoop.RunUntilIdle();
    CHECK(fired == 0);
    CHECK_FALSE(eventLoop.HasPendingTasks());

    // Tasks without a document still run.
    bool ranOther = false;
    eventLoop.QueueTask(html::TaskSource::DOMManipulation, [&] { ranOther = true; });
    eventLoop.RunUntilIdle();
    CHECK(ranOther);

    SECTION("resuming for another reason doesn't resume it")
    {
      document.ResumeScheduledTasks(dom::ReasonForSuspension::PageWillBeSuspended);
      CHECK(document.ScheduledTasksAreSuspended());
      eventLoop.RunUntilIdle();
      CHECK(fired == 0);
    }

    SECTION("resuming runs them")
    {
      document.Resume(Reason);
      CHECK_FALSE(document.IsSuspended());
      CHECK(eventLoop.HasPendingTasks());
      eventLoop.RunUntilIdle();
      CHECK(fired == 1);
    }

    SECTION("stopping drops them")
    {
      document.StopActiveDOMObjects();
      CHECK(document.ActiveDOMObjectsAreStopped());
      document.Resume(Reason);
      eventLoop.RunUntilIdle();
      CHECK(fired == 0);
    }
  }

  TEST_CASE("A suspended document's timers keep the time they had left", "[DOM][Document][Suspension]")
  {
    SuspensionFixture fixture;
    auto &document = fixture.Document();
    auto &window = fixture.Window();
    List<std::string> fired;
    (void)window.SetTimeout([&] { fired.push_back("timeout"); }, 100);
    (void)window.SetInterval([&] { fired.push_back("interval"); }, 40);

    Advance(50ms);
    CHECK(fired == List<std::string> {"interval"});

    document.Suspend(Reason);
    // A timer installed while the document is suspended waits too.
    (void)window.SetTimeout([&] { fired.push_back("installed"); }, 10);
    Advance(200ms);
    CHECK(fired == List<std::string> {"interval"});

    document.Resume(Reason);
    // The interval had 40ms left (it was due again 40ms after it fired, at 50ms), the timeout 50ms and the
    // new timer 10ms.
    Advance(15ms);
    CHECK(fired == List<std::string> {"interval", "installed"});
    Advance(30ms);
    CHECK(fired == List<std::string> {"interval", "installed", "interval"});
    Advance(10ms);
    CHECK(fired == List<std::string> {"interval", "installed", "interval", "timeout"});
  }

  TEST_CASE("A suspended document's animation frame callbacks wait", "[DOM][Document][Suspension]")
  {
    SuspensionFixture fixture;
    auto &document = fixture.Document();
    uint32 frames = 0;
    (void)fixture.Window().RequestAnimationFrame([&](double) { ++frames; });

    document.Suspend(Reason);
    fixture.TestPage->UpdateRendering();
    CHECK(frames == 0);

    document.Resume(Reason);
    fixture.TestPage->UpdateRendering();
    CHECK(frames == 1);
  }

  TEST_CASE("A document's back/forward cache state", "[DOM][Document][Suspension]")
  {
    SuspensionFixture fixture;
    auto &document = fixture.Document();
    CHECK(document.GetBackForwardCacheState() == dom::BackForwardCacheState::NotInBackForwardCache);
    document.SetBackForwardCacheState(dom::BackForwardCacheState::AboutToEnterBackForwardCache);
    CHECK(document.GetBackForwardCacheState() == dom::BackForwardCacheState::AboutToEnterBackForwardCache);
    document.SetBackForwardCacheState(dom::BackForwardCacheState::InBackForwardCache);
    CHECK(document.GetBackForwardCacheState() == dom::BackForwardCacheState::InBackForwardCache);
    document.SetBackForwardCacheState(dom::BackForwardCacheState::NotInBackForwardCache);
    CHECK(document.GetBackForwardCacheState() == dom::BackForwardCacheState::NotInBackForwardCache);
  }

  TEST_CASE("A document in the back/forward cache doesn't recalculate its style",
            "[DOM][Document][Suspension]")
  {
    SuspensionFixture fixture;
    auto &document = fixture.Document();
    auto &eventLoop = html::EventLoop::Current();
    document.UpdateStyleIfNeeded();

    document.SetBackForwardCacheState(dom::BackForwardCacheState::InBackForwardCache);
    (void)document.Body()->FirstElementChild()->SetAttribute(u8"style", u8"color: blue");
    eventLoop.RunUntilIdle();
    CHECK(document.HasPendingStyleRecalc());

    // Leaving the cache schedules it.
    document.SetBackForwardCacheState(dom::BackForwardCacheState::NotInBackForwardCache);
    eventLoop.RunUntilIdle();
    CHECK_FALSE(document.HasPendingStyleRecalc());
  }
}

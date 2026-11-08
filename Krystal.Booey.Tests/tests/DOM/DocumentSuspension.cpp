#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLLinkElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"

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

    constexpr auto Reason = dom::ReasonForSuspension::PageWillBeSuspended;
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
      document.ResumeScheduledTasks(dom::ReasonForSuspension::WillDeferLoading);
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
    List<std::string> fired;
    auto timeout = document.StartTimer(100ms, [&] { fired.push_back("timeout"); });
    auto interval = document.StartRepeatingTimer(40ms, [&] { fired.push_back("interval"); });

    Advance(50ms);
    CHECK(fired == List<std::string> {"interval"});

    document.Suspend(Reason);
    // A timer installed while the document is suspended waits too.
    auto installed = document.StartTimer(10ms, [&] { fired.push_back("installed"); });
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

  TEST_CASE("A suspended document's web fonts start loading once it resumes", "[DOM][Document][Suspension]")
  {
    SuspensionFixture fixture;
    auto &document = fixture.Document();
    // The bytes don't matter: only whether the font is requested.
    fixture.Strategy.AddText(u8"https://example.com/font.ttf", u8"not a font", u8"font/ttf");

    document.Suspend(Reason);
    auto style = document.CreateElement(u8"style").ReleaseValue();
    (void)style->TextContent(u8"@font-face { font-family: web; src: url(font.ttf) } p { font-family: web }");
    (void)document.Body()->AppendChild(*style);
    (void)document.UpdateLayout();
    html::EventLoop::Current().RunUntilIdle();
    RunNetworkingTasks();
    auto requested = [&]
    {
      return std::ranges::count(fixture.Strategy.Requests, utf8_string(u8"https://example.com/font.ttf"));
    };
    CHECK(requested() == 0);

    document.Resume(Reason);
    html::EventLoop::Current().RunUntilIdle();
    RunNetworkingTasks();
    CHECK(requested() == 1);
  }

  TEST_CASE("A suspended document's subresource loads finish once it resumes", "[DOM][Document][Suspension]")
  {
    SuspensionFixture fixture;
    auto &document = fixture.Document();
    fixture.Strategy.AddText(u8"https://example.com/sheet.css", u8"p { color: green }", u8"text/css");

    document.Suspend(Reason);
    auto link = document.CreateElement(u8"link").ReleaseValue();
    (void)link->SetAttribute(u8"rel", u8"stylesheet");
    (void)link->SetAttribute(u8"href", u8"sheet.css");
    (void)document.Body()->AppendChild(*link);
    html::EventLoop::Current().RunUntilIdle();
    RunNetworkingTasks();
    auto &linkElement = Downcast<html::HTMLLinkElement>(*link);
    CHECK_FALSE(linkElement.Sheet());

    document.Resume(Reason);
    html::EventLoop::Current().RunUntilIdle();
    RunNetworkingTasks();
    CHECK(linkElement.Sheet());
  }

  TEST_CASE("A suspended document's AbortSignal.timeout() waits for it", "[DOM][Document][Suspension]")
  {
    SuspensionFixture fixture;
    auto &document = fixture.Document();

    auto signal = dom::AbortSignal::Timeout(document, 100);
    Advance(60ms);
    document.Suspend(Reason);
    Advance(100ms);
    CHECK_FALSE(signal->Aborted());

    // The time it had left runs once the document resumes.
    document.Resume(Reason);
    Advance(39ms);
    CHECK_FALSE(signal->Aborted());
    Advance(1ms);
    CHECK(signal->Aborted());
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

}

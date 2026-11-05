#include "Krystal.Booey/DOM/Scheduling/DocumentTimer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    /// @brief Runs the event loop with the host's time moved on by @p delta.
    void AdvanceTimers(MonotonicTime::duration delta) noexcept
    {
      auto &eventLoop = html::EventLoop::Current();
      eventLoop.Run(eventLoop.Now() + delta);
    }

    struct TimerFixture : LoadFixture
    {
      TimerFixture() noexcept
      {
        LoadMarkup(u8"<!DOCTYPE html><p>Page</p>", u8"https://example.com/page.html");
        html::EventLoop::Current().RunUntilIdle();
      }
    };
  }

  TEST_CASE("A document's timers", "[DOM][Scheduling][DocumentTimer]")
  {
    TimerFixture fixture;
    auto &document = fixture.Document();
    List<std::string> order;

    SECTION("one-shot timers run as tasks after their delay, in order")
    {
      auto late = document.StartTimer(20ms, [&] { order.push_back("late"); });
      auto early = document.StartTimer(10ms, [&] { order.push_back("early"); });
      auto now = document.StartTimer(-5ms, [&] { order.push_back("now"); });
      CHECK(late.IsActive());
      CHECK(order.empty());

      AdvanceTimers(0ms);
      CHECK(order == List<std::string> {"now"});
      CHECK_FALSE(now.IsActive());
      AdvanceTimers(10ms);
      CHECK(order == List<std::string> {"now", "early"});
      AdvanceTimers(10ms);
      CHECK(order == List<std::string> {"now", "early", "late"});
      CHECK_FALSE(late);
    }

    SECTION("a stopped timer doesn't run")
    {
      auto timer = document.StartTimer(5ms, [&] { order.push_back("stopped"); });
      timer.Stop();
      CHECK_FALSE(timer.IsActive());
      timer.Stop();
      AdvanceTimers(10ms);
      CHECK(order.empty());
    }

    SECTION("destroying the handle stops the timer, unless it was released")
    {
      {
        auto dropped = document.StartTimer(5ms, [&] { order.push_back("dropped"); });
        auto released = document.StartTimer(5ms, [&] { order.push_back("released"); });
        released.Release();
        CHECK_FALSE(released.IsActive());
      }
      AdvanceTimers(10ms);
      CHECK(order == List<std::string> {"released"});
    }

    SECTION("moving the handle moves the timer, and assigning stops the old one")
    {
      auto first = document.StartTimer(5ms, [&] { order.push_back("first"); });
      auto moved = krys::move(first);
      CHECK_FALSE(first.IsActive());
      CHECK(moved.IsActive());
      moved = document.StartTimer(5ms, [&] { order.push_back("second"); });
      AdvanceTimers(10ms);
      CHECK(order == List<std::string> {"second"});
    }

    SECTION("repeating timers repeat until stopped, even from their own action")
    {
      dom::DocumentTimerHandle timer;
      int32 count = 0;
      timer = document.StartRepeatingTimer(10ms,
                                           [&]
                                           {
                                             if (++count == 3)
                                             {
                                               timer.Stop();
                                             }
                                           });
      for (int32 i = 0; i < 6; ++i)
      {
        AdvanceTimers(10ms);
      }
      CHECK(count == 3);
    }

    SECTION("timers started from a timer's action aren't clamped")
    {
      int32 runs = 0;
      List<dom::DocumentTimerHandle> timers;
      Func<void()> nest;
      nest = [&]
      {
        if (++runs < 12)
        {
          timers.push_back(document.StartTimer(0ms, Func<void()>(nest)));
        }
      };
      timers.push_back(document.StartTimer(0ms, Func<void()>(nest)));
      // All twelve levels run without the host's time moving on.
      html::EventLoop::Current().RunUntilIdle();
      CHECK(runs == 12);
      timers.clear();
    }

    SECTION("a document leaving its frame stops its timers")
    {
      auto timeout = document.StartTimer(5ms, [&] { order.push_back("timeout"); });
      auto interval = document.StartRepeatingTimer(5ms, [&] { order.push_back("interval"); });
      fixture.TestPage->MainFrame().SetDocument(nullptr);
      CHECK_FALSE(timeout.IsActive());
      CHECK_FALSE(interval.IsActive());
      AdvanceTimers(10ms);
      CHECK(order.empty());
    }
  }
}

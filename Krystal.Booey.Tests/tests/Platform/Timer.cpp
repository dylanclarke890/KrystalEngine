#include "Krystal.Booey/Platform/Timer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Platform/ThreadTimers.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    /// @brief Runs the event loop @p delta of host time after the last run.
    void Advance(MonotonicTime::duration delta) noexcept
    {
      auto &loop = html::EventLoop::Current();
      loop.Run(loop.Now() + delta);
    }
  }

  TEST_CASE("Timer: one-shot timers fire once, when they're due", "[Platform][Timer]")
  {
    int32 fired = 0;
    Timer timer([&] { ++fired; });
    CHECK_FALSE(timer.IsActive());

    timer.StartOneShot(100ms);
    CHECK(timer.IsActive());
    CHECK(timer.NextFireInterval() == 100ms);

    Advance(99ms);
    CHECK(fired == 0);
    CHECK(timer.NextFireInterval() == 1ms);
    Advance(1ms);
    CHECK(fired == 1);
    CHECK_FALSE(timer.IsActive());
    Advance(1s);
    CHECK(fired == 1);

    SECTION("restarting moves the fire time")
    {
      timer.StartOneShot(10ms);
      timer.StartOneShot(50ms);
      Advance(10ms);
      CHECK(fired == 1);
      Advance(40ms);
      CHECK(fired == 2);
    }

    SECTION("stopping cancels it")
    {
      timer.StartOneShot(10ms);
      timer.Stop();
      CHECK_FALSE(timer.IsActive());
      Advance(1s);
      CHECK(fired == 1);
    }
  }

  TEST_CASE("Timer: repeating timers", "[Platform][Timer]")
  {
    int32 fired = 0;
    Timer timer([&] { ++fired; });
    timer.StartRepeating(10ms);
    CHECK(timer.RepeatInterval() == 10ms);

    Advance(10ms);
    CHECK(fired == 1);
    CHECK(timer.IsActive());
    Advance(10ms);
    CHECK(fired == 2);

    // A late run fires it once, then it repeats from then.
    Advance(35ms);
    CHECK(fired == 3);
    CHECK(timer.NextFireInterval() == 10ms);

    timer.Stop();
    Advance(1s);
    CHECK(fired == 3);
  }

  TEST_CASE("Timer: timers fire soonest first, then in the order they were started", "[Platform][Timer]")
  {
    List<int32> order;
    Timer late([&] { order.push_back(3); });
    Timer first([&] { order.push_back(1); });
    Timer second([&] { order.push_back(2); });
    late.StartOneShot(20ms);
    first.StartOneShot(10ms);
    second.StartOneShot(10ms);

    Advance(1s);
    CHECK(order == List<int32> {1, 2, 3});
  }

  TEST_CASE("Timer: the host's time", "[Platform][Timer]")
  {
    auto &loop = html::EventLoop::Current();
    int32 fired = 0;
    Timer timer([&] { ++fired; });

    SECTION("doesn't go backwards")
    {
      auto now = loop.Now();
      loop.Run(now - 1s);
      CHECK(loop.Now() == now);
    }

    SECTION("a zero-delay timer is due at once, but one started while timers fire waits for the next run")
    {
      Timer restarting([&] { timer.StartOneShot({}); });
      restarting.StartOneShot({});
      loop.RunUntilIdle();
      // The loop runs again until idle, so the timer it started fires too.
      CHECK(fired == 1);

      timer.StartOneShot({});
      Timer again([&] { timer.StartOneShot({}); });
      again.StartOneShot({});
      loop.Run(loop.Now());
      CHECK(fired == 2);
      CHECK(timer.IsActive());
      loop.Run(loop.Now());
      CHECK(fired == 3);
    }

    SECTION("a destroyed timer stops")
    {
      auto doomed = CreateUnique<Timer>([&] { ++fired; });
      doomed->StartOneShot(10ms);
      doomed = nullptr;
      Advance(1s);
      CHECK(fired == 0);
    }

    SECTION("a timer may destroy itself as it fires")
    {
      UniquePtr<Timer> self;
      self = CreateUnique<Timer>(
        [&]
        {
          ++fired;
          self = nullptr;
        });
      Timer other([&] { ++fired; });
      self->StartOneShot(10ms);
      other.StartOneShot(10ms);
      Advance(10ms);
      CHECK(fired == 2);
    }
  }

  TEST_CASE("EventLoop: tasks after a timeout, the budget and the next wake time", "[HTML][EventLoop][Timer]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    List<int32> order;

    loop.ScheduleTask(50ms, html::TaskSource::Timer, [&] { order.push_back(1); });
    REQUIRE(loop.NextWakeTime());
    CHECK(*loop.NextWakeTime() <= loop.Now() + 50ms);
    Advance(49ms);
    CHECK(order.empty());
    Advance(1ms);
    CHECK(order == List<int32> {1});

    // Queued tasks are work now.
    loop.QueueTask(html::TaskSource::DOMManipulation, [&] { order.push_back(2); });
    CHECK(loop.NextWakeTime() == loop.Now());

    // A spent budget leaves tasks for the next run.
    loop.QueueTask(html::TaskSource::DOMManipulation, [&] { order.push_back(3); });
    loop.Run(loop.Now(), MonotonicTime::duration::zero());
    CHECK(order == List<int32> {1});
    loop.Run(loop.Now());
    CHECK(order == List<int32> {1, 2, 3});
  }
}

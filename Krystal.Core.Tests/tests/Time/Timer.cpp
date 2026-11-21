#include "Krystal.Core/Time/Timer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Time/ThreadTimers.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <thread>

namespace krys::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    /// @brief Moves this thread's time on by @p delta and fires the timers that are due.
    void Advance(MonotonicTime::duration delta) noexcept
    {
      auto &timers = ThreadTimers::Current();
      timers.SetCurrentTime(timers.CurrentTime() + delta);
      (void)timers.FireTimersDue();
    }
  }

  TEST_CASE("Timer: one-shot timers fire once, when they're due", "[Time][Timer]")
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

  TEST_CASE("Timer: repeating timers", "[Time][Timer]")
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

  TEST_CASE("Timer: timers fire soonest first, then in the order they were started", "[Time][Timer]")
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

  TEST_CASE("Timer: the host's time", "[Time][Timer]")
  {
    auto &timers = ThreadTimers::Current();
    int32 fired = 0;
    Timer timer([&] { ++fired; });

    SECTION("doesn't go backwards")
    {
      auto now = timers.CurrentTime();
      timers.SetCurrentTime(now - 1s);
      CHECK(timers.CurrentTime() == now);
    }

    SECTION("a zero-delay timer is due at once, but one started while timers fire waits for the next run")
    {
      Timer restarting([&] { timer.StartOneShot({}); });
      restarting.StartOneShot({});
      CHECK(timers.FireTimersDue());
      CHECK(fired == 0);
      CHECK(timer.IsActive());
      CHECK(timers.FireTimersDue());
      CHECK(fired == 1);
      CHECK_FALSE(timers.FireTimersDue());
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

  TEST_CASE("Timer: each thread has its own timers", "[Time][Timer]")
  {
    auto &mainTimers = ThreadTimers::Current();
    auto mainNextFireTime = mainTimers.NextFireTime();

    bool separate = false;
    bool firedOnItsThread = false;
    std::thread thread(
      [&]
      {
        auto &timers = ThreadTimers::Current();
        separate = &timers != &mainTimers;
        Timer timer([&] { firedOnItsThread = true; });
        timer.StartOneShot(10ms);
        timers.SetCurrentTime(timers.CurrentTime() + 10ms);
        (void)timers.FireTimersDue();
      });
    thread.join();

    CHECK(separate);
    CHECK(firedOnItsThread);
    // The other thread's timer never joined this thread's set.
    CHECK(mainTimers.NextFireTime() == mainNextFireTime);
  }
}

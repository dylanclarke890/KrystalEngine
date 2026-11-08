#include "Krystal.Threading/JobSystem.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Threading/MainThread.hpp"
#include <atomic>
#include <latch>
#include <mutex>
#include <thread>

namespace krys::tests
{
  namespace
  {
    /// @brief Counts a latch down when it's destroyed.
    struct CountDownOnDestruction
    {
      std::latch *Latch;

      explicit CountDownOnDestruction(std::latch &latch) noexcept : Latch(&latch)
      {
      }

      ~CountDownOnDestruction() noexcept
      {
        Latch->count_down();
      }
    };
  }

  TEST_CASE("JobSystem: workers", "[Threading]")
  {
    CHECK(JobSystem::DefaultWorkerCount() >= 1);
    CHECK(JobSystem::DefaultWorkerCount() == std::max(std::thread::hardware_concurrency(), 2u) - 1);
    CHECK(JobSystem().WorkerCount() == JobSystem::DefaultWorkerCount());
    CHECK(JobSystem(3).WorkerCount() == 3);
    // There's always one.
    CHECK(JobSystem(0).WorkerCount() == 1);
  }

  TEST_CASE("JobSystem: every job runs once, on a worker", "[Threading]")
  {
    constexpr int JobCount = 1'000;
    std::atomic<int> runs {0};
    std::atomic<int> onMain {0};
    {
      JobSystem jobs(4);
      for (int i = 0; i < JobCount; ++i)
      {
        jobs.Dispatch(
          [&]()
          {
            ++runs;
            if (IsMainThread())
            {
              ++onMain;
            }
          });
      }
      // Destruction lets the Normal jobs queued run.
    }
    CHECK(runs == JobCount);
    CHECK(onMain == 0);
  }

  TEST_CASE("JobSystem: a job can dispatch another", "[Threading]")
  {
    std::latch done(1);
    JobSystem jobs(2);
    jobs.Dispatch([&]() { jobs.Dispatch([&]() { done.count_down(); }); });
    done.wait();
  }

  TEST_CASE("JobSystem: a free worker takes Normal jobs before Background ones", "[Threading]")
  {
    JobSystem jobs(1);
    std::latch started(1);
    std::latch release(1);
    std::latch done(4);
    std::mutex mutex;
    List<int> order;
    auto record = [&](int value)
    {
      return [&, value]()
      {
        {
          std::scoped_lock lock(mutex);
          order.push_back(value);
        }
        done.count_down();
      };
    };

    // The only worker is busy while the others queue.
    jobs.Dispatch(
      [&]()
      {
        started.count_down();
        release.wait();
      });
    started.wait();
    jobs.Dispatch(record(1), JobPriority::Background);
    jobs.Dispatch(record(2));
    jobs.Dispatch(record(3), JobPriority::Background);
    jobs.Dispatch(record(4));
    release.count_down();
    done.wait();

    std::scoped_lock lock(mutex);
    CHECK(order == List<int> {2, 4, 1, 3});
  }

  TEST_CASE("JobSystem: shutting down runs the Normal jobs left and drops the Background ones", "[Threading]")
  {
    std::atomic<int> normalRuns {0};
    std::atomic<bool> backgroundRan {false};
    // The only worker waits until the Background job is destroyed, so it can't have started it.
    std::latch backgroundDestroyed(1);
    {
      JobSystem jobs(1);
      jobs.Dispatch([&]() { backgroundDestroyed.wait(); });
      jobs.Dispatch([&, guard = CreateUnique<CountDownOnDestruction>(backgroundDestroyed)]()
                    { backgroundRan = true; }, JobPriority::Background);
      jobs.Dispatch([&]() { ++normalRuns; });
      jobs.Dispatch([&]() { ++normalRuns; });
    }
    CHECK(normalRuns == 2);
    CHECK_FALSE(backgroundRan);
  }

  TEST_CASE("JobSystem: a job's completion runs on its inbox's owner, with its result", "[Threading]")
  {
    TaskInbox inbox;
    List<int> results;
    bool voidCompleted = false;
    std::atomic<bool> jobOnMain {true};
    UniquePtr<int> owned;
    {
      JobSystem jobs(2);
      jobs.Dispatch(
        [&]()
        {
          jobOnMain = IsMainThread();
          return 6 * 7;
        },
        inbox,
        [&](int result)
        {
          CHECK(IsMainThread());
          results.push_back(result);
        });
      // What can only be moved can be the result.
      jobs.Dispatch([]() { return CreateUnique<int>(5); }, inbox,
                    [&](UniquePtr<int> result) { owned = krys::move(result); });
      jobs.Dispatch([]() {}, inbox, [&]() { voidCompleted = true; });
    }

    // Nothing completes until the inbox is drained.
    CHECK(results.empty());
    CHECK_FALSE(owned);
    CHECK_FALSE(voidCompleted);
    CHECK(inbox.Drain() == 3);
    CHECK(results == List<int> {42});
    REQUIRE(owned);
    CHECK(*owned == 5);
    CHECK(voidCompleted);
    CHECK_FALSE(jobOnMain);
  }
}

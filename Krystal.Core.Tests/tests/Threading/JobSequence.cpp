#include "Krystal.Core/Threading/JobSequence.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Threading/JobSystem.hpp"
#include "Krystal.Core/Threading/MainThread.hpp"
#include "Krystal.Core/Threading/ManualWorkQueue.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <atomic>
#include <latch>

namespace krys::tests
{
  TEST_CASE("JobSequence: its jobs run one at a time, in order, on the queue's workers", "[Threading]")
  {
    constexpr int JobCount = 1'000;
    List<int> order;
    std::atomic<int> running {0};
    std::atomic<bool> overlapped {false};
    std::atomic<bool> onMain {false};
    std::latch done(JobCount);
    {
      JobSystem jobs(4);
      JobSequence sequence(jobs);
      for (int i = 0; i < JobCount; ++i)
      {
        // The list isn't locked: the sequence keeps its jobs from overlapping, and each sees what the ones
        // before it wrote.
        sequence.Dispatch(
          [&, i]()
          {
            if (++running > 1)
            {
              overlapped = true;
            }
            if (IsMainThread())
            {
              onMain = true;
            }
            order.push_back(i);
            --running;
            done.count_down();
          });
      }
      done.wait();
    }

    CHECK_FALSE(overlapped);
    CHECK_FALSE(onMain);
    REQUIRE(order.size() == static_cast<size_t>(JobCount));
    bool inOrder = true;
    for (int i = 0; i < JobCount; ++i)
    {
      inOrder &= order[static_cast<size_t>(i)] == i;
    }
    CHECK(inOrder);
  }

  TEST_CASE("JobSequence: it queues one job at a time on its queue", "[Threading]")
  {
    ManualWorkQueue queue;
    List<int> order;
    JobSequence sequence(queue);
    sequence.Dispatch([&]() { order.push_back(1); });
    sequence.Dispatch([&]() { order.push_back(2); });
    sequence.Dispatch([&]() { order.push_back(3); });
    CHECK(queue.PendingCount() == 1u);

    // Each job queues the next: three runs, one job each.
    CHECK(queue.RunNext());
    CHECK(order == List<int> {1});
    CHECK(queue.PendingCount() == 1u);
    CHECK(queue.RunAll() == 2u);
    CHECK(order == List<int> {1, 2, 3});

    // A job dispatched to it after it's emptied queues again.
    sequence.Dispatch([&]() { order.push_back(4); });
    CHECK(queue.RunAll() == 1u);
    CHECK(order == List<int> {1, 2, 3, 4});
  }

  TEST_CASE("JobSequence: its jobs take its traits", "[Threading]")
  {
    ManualWorkQueue queue;
    List<int> order;
    JobSequence background(queue, {.Priority = JobPriority::Background});
    background.Dispatch([&]() { order.push_back(1); });
    queue.Dispatch([&]() { order.push_back(2); });
    CHECK(queue.RunAll() == 2u);
    CHECK(order == List<int> {2, 1});
  }

  TEST_CASE("JobSequence: its jobs run after it's destroyed", "[Threading]")
  {
    ManualWorkQueue queue;
    List<int> order;
    {
      JobSequence sequence(queue);
      sequence.Dispatch([&]() { order.push_back(1); });
      sequence.Dispatch([&]() { order.push_back(2); });
    }
    CHECK(queue.RunAll() == 2u);
    CHECK(order == List<int> {1, 2});
  }
}

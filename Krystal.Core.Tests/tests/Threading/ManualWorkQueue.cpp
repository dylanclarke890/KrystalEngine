#include "Krystal.Core/Threading/ManualWorkQueue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <thread>

namespace krys::tests
{
  TEST_CASE("ManualWorkQueue: jobs run when the test says, on its thread", "[Threading]")
  {
    ManualWorkQueue queue;
    List<int> order;
    std::thread::id ranOn;
    queue.Dispatch([&]() { order.push_back(1); }, {.Priority = JobPriority::Background});
    queue.Dispatch(
      [&]()
      {
        order.push_back(2);
        ranOn = std::this_thread::get_id();
      });
    CHECK(queue.PendingCount() == 2);
    CHECK(order.empty());

    // Normal jobs first.
    CHECK(queue.RunNext());
    CHECK(order == List<int> {2});
    CHECK(ranOn == std::this_thread::get_id());
    CHECK(queue.RunNext());
    CHECK(order == List<int> {2, 1});
    CHECK_FALSE(queue.RunNext());
  }

  TEST_CASE("ManualWorkQueue: running all runs the jobs that jobs dispatch", "[Threading]")
  {
    ManualWorkQueue queue;
    int runs = 0;
    queue.Dispatch(
      [&]()
      {
        ++runs;
        queue.Dispatch([&]() { ++runs; });
      });
    CHECK(queue.RunAll() == 2);
    CHECK(runs == 2);
    CHECK(queue.PendingCount() == 0);
  }

  TEST_CASE("ManualWorkQueue: a job's completion goes to its inbox", "[Threading]")
  {
    ManualWorkQueue queue;
    WorkQueue &workQueue = queue;
    TaskInbox inbox;
    UniquePtr<int> result;
    workQueue.Dispatch([]() { return CreateUnique<int>(9); }, inbox,
                       [&](UniquePtr<int> value) { result = krys::move(value); });
    CHECK(inbox.Drain() == 0);

    CHECK(queue.RunAll() == 1);
    CHECK_FALSE(result);
    CHECK(inbox.Drain() == 1);
    REQUIRE(result);
    CHECK(*result == 9);
  }

  TEST_CASE("ManualWorkQueue: clearing destroys the jobs waiting, unrun", "[Threading]")
  {
    ManualWorkQueue queue;
    bool ran = false;
    queue.Dispatch([&]() { ran = true; });
    queue.Clear();
    CHECK(queue.PendingCount() == 0);
    CHECK(queue.RunAll() == 0);
    CHECK_FALSE(ran);
  }
}

#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE("EventLoop", "[HTML][EventLoop]")
  {
    EventLoop loop;
    List<int> order;

    SECTION("Tasks run in the order they were queued")
    {
      loop.QueueTask(TaskSource::DOMManipulation, [&] { order.push_back(1); });
      loop.QueueTask(TaskSource::Timer, [&] { order.push_back(2); });
      REQUIRE(loop.HasPendingTasks());

      REQUIRE(loop.RunNextTask());
      REQUIRE(order == List<int> {1});

      REQUIRE(loop.RunNextTask());
      REQUIRE_FALSE(loop.RunNextTask());
      REQUIRE(order == List<int> {1, 2});
    }

    SECTION("Microtasks run after each task, before the next one")
    {
      loop.QueueTask(TaskSource::DOMManipulation,
                     [&]
                     {
                       order.push_back(1);
                       loop.QueueMicrotask([&] { order.push_back(2); });
                     });
      loop.QueueTask(TaskSource::DOMManipulation, [&] { order.push_back(3); });

      loop.RunUntilIdle();
      REQUIRE(order == List<int> {1, 2, 3});
    }

    SECTION("Microtasks queued by microtasks run in the same checkpoint")
    {
      loop.QueueMicrotask(
        [&]
        {
          order.push_back(1);
          loop.QueueMicrotask([&] { order.push_back(2); });
        });

      loop.PerformMicrotaskCheckpoint();
      REQUIRE(order == List<int> {1, 2});
      REQUIRE_FALSE(loop.HasPendingMicrotasks());
    }

    SECTION("A checkpoint inside a checkpoint does nothing")
    {
      loop.QueueMicrotask(
        [&]
        {
          loop.QueueMicrotask([&] { order.push_back(2); });
          loop.PerformMicrotaskCheckpoint();
          order.push_back(1);
        });

      loop.PerformMicrotaskCheckpoint();
      REQUIRE(order == List<int> {1, 2});
    }

    SECTION("Tasks queued by tasks run until idle")
    {
      loop.QueueTask(TaskSource::Networking,
                     [&]
                     {
                       order.push_back(1);
                       loop.QueueTask(TaskSource::Networking, [&] { order.push_back(2); });
                     });

      loop.RunUntilIdle();
      REQUIRE(order == List<int> {1, 2});
      REQUIRE_FALSE(loop.HasPendingTasks());
    }
  }
}

#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Core/Threading/JobSystem.hpp"
#include "Krystal.Core/Threading/ManualWorkQueue.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE("EventLoop", "[HTML][EventLoop][html:event-loops][html:queue-a-task][html:queue-a-microtask]"
                         "[html:perform-a-microtask-checkpoint][html:event-loop-processing-model]")
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

    SECTION("A document's tasks are dropped once it's gone, unless they're to run regardless")
    {
      RefPtr<dom::Document> document = CreateRef<dom::Document>();
      loop.QueueTask(TaskSource::Networking, *document, [&] { order.push_back(1); });
      loop.QueueTask(
        TaskSource::Networking, *document, [&] { order.push_back(2); }, StoppedDocumentTask::Run);
      document = nullptr;

      loop.RunUntilIdle();
      REQUIRE(order == List<int> {2});
    }
  }

  TEST_CASE("EventLoop: jobs post back to its inbox", "[HTML][EventLoop][html:event-loops]")
  {
    EventLoop loop;
    List<utf8_string> order;

    SECTION("a completion runs as the loop runs, and queues a task for what follows")
    {
      ManualWorkQueue queue;
      int received = 0;
      queue.Dispatch([]() { return 42; }, loop.Inbox(),
                     [&](int result)
                     {
                       received = result;
                       order.push_back(u8"completion");
                       loop.QueueMicrotask([&] { order.push_back(u8"microtask"); });
                       loop.QueueTask(TaskSource::DOMManipulation, [&] { order.push_back(u8"task"); });
                     });

      REQUIRE(queue.RunAll() == 1);
      // The completion waits in the inbox, which wakes the loop.
      CHECK(order.empty());
      CHECK(loop.NextWakeTime() == loop.Now());

      loop.RunUntilIdle();
      CHECK(order == List<utf8_string> {u8"completion", u8"microtask", u8"task"});
      CHECK(received == 42);
    }

    SECTION("from the job system's workers")
    {
      {
        JobSystem jobs(2);
        for (int i = 0; i < 3; ++i)
        {
          jobs.Dispatch([]() {}, loop.Inbox(), [&] { order.push_back(u8"done"); });
        }
        // The workers finish their jobs as the system goes.
      }
      CHECK(order.empty());
      loop.Run(loop.Now());
      CHECK(order == List<utf8_string> {u8"done", u8"done", u8"done"});
    }
  }
}

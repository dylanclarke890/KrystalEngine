#include "Krystal.Core/Threading/TaskInbox.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Threading/Thread.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <atomic>
#include <thread>

namespace krys::tests
{
  namespace
  {
    /// @brief Sets a flag when it's destroyed.
    struct DestructionFlag
    {
      bool *Destroyed;

      explicit DestructionFlag(bool &destroyed) noexcept : Destroyed(&destroyed)
      {
      }

      ~DestructionFlag() noexcept
      {
        *Destroyed = true;
      }
    };
  }

  TEST_CASE("TaskInbox: tasks run when the owner drains, in the order they came", "[Threading]")
  {
    TaskInbox inbox;
    List<int> order;
    inbox.Post([&]() { order.push_back(1); });
    inbox.Post([&]() { order.push_back(2); });
    CHECK(order.empty());
    CHECK(inbox.HasTasks());

    CHECK(inbox.Drain() == 2);
    CHECK(order == List<int> {1, 2});
    CHECK_FALSE(inbox.HasTasks());
    CHECK(inbox.Drain() == 0);
  }

  TEST_CASE("TaskInbox: a task posted while draining runs at the next drain", "[Threading]")
  {
    TaskInbox inbox;
    int runs = 0;
    inbox.Post(
      [&]()
      {
        ++runs;
        inbox.Post([&]() { ++runs; });
      });
    CHECK(inbox.Drain() == 1);
    CHECK(runs == 1);
    CHECK(inbox.Drain() == 1);
    CHECK(runs == 2);
  }

  TEST_CASE("TaskInbox: other threads post, the owner runs", "[Threading]")
  {
    TaskInbox inbox;
    std::atomic<uint32> wakes {0};
    inbox.SetWakeHandler([&]() { ++wakes; });

    constexpr int PostsEach = 100;
    List<std::thread::id> ranOn;
    List<int> fromFirst;
    {
      Thread first(u8"First",
                   [&]()
                   {
                     for (int i = 0; i < PostsEach; ++i)
                     {
                       inbox.Post(
                         [&, i]()
                         {
                           fromFirst.push_back(i);
                           ranOn.push_back(std::this_thread::get_id());
                         });
                     }
                   });
      Thread second(u8"Second",
                    [&]()
                    {
                      for (int i = 0; i < PostsEach; ++i)
                      {
                        inbox.Post([&]() { ranOn.push_back(std::this_thread::get_id()); });
                      }
                    });
    }

    CHECK(wakes == 2 * PostsEach);
    CHECK(inbox.Drain() == 2 * PostsEach);
    REQUIRE(ranOn.size() == 2uz * PostsEach);
    for (auto id : ranOn)
    {
      CHECK(id == std::this_thread::get_id());
    }
    // Each thread's tasks keep their order.
    REQUIRE(fromFirst.size() == PostsEach);
    for (int i = 0; i < PostsEach; ++i)
    {
      CHECK(fromFirst[static_cast<size_t>(i)] == i);
    }
  }

  TEST_CASE("TaskInbox: tasks left when it goes are destroyed unrun", "[Threading]")
  {
    bool ran = false;
    bool destroyed = false;
    {
      TaskInbox inbox;
      inbox.Post([&ran, flag = CreateUnique<DestructionFlag>(destroyed)]() { ran = true; });
    }
    CHECK_FALSE(ran);
    CHECK(destroyed);
  }
}

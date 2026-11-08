#include "Krystal.Threading/TaskTarget.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Threading/Thread.hpp"
#include <atomic>
#include <thread>

namespace krys::threads::tests
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

  TEST_CASE("TaskTarget: tasks run when the owner drains, in the order they came", "[Threading]")
  {
    TaskTarget target;
    List<int> order;
    target.Post([&]() { order.push_back(1); });
    target.Post([&]() { order.push_back(2); });
    CHECK(order.empty());
    CHECK(target.HasTasks());

    CHECK(target.Drain() == 2);
    CHECK(order == List<int> {1, 2});
    CHECK_FALSE(target.HasTasks());
    CHECK(target.Drain() == 0);
  }

  TEST_CASE("TaskTarget: a task posted while draining runs at the next drain", "[Threading]")
  {
    TaskTarget target;
    int runs = 0;
    target.Post(
      [&]()
      {
        ++runs;
        target.Post([&]() { ++runs; });
      });
    CHECK(target.Drain() == 1);
    CHECK(runs == 1);
    CHECK(target.Drain() == 1);
    CHECK(runs == 2);
  }

  TEST_CASE("TaskTarget: other threads post, the owner runs", "[Threading]")
  {
    TaskTarget target;
    std::atomic<uint32> wakes {0};
    target.SetWakeHandler([&]() { ++wakes; });

    constexpr int PostsEach = 100;
    List<std::thread::id> ranOn;
    List<int> fromFirst;
    {
      Thread first(u8"First",
                   [&]()
                   {
                     for (int i = 0; i < PostsEach; ++i)
                     {
                       target.Post(
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
                        target.Post([&]() { ranOn.push_back(std::this_thread::get_id()); });
                      }
                    });
    }

    CHECK(wakes == 2 * PostsEach);
    CHECK(target.Drain() == 2 * PostsEach);
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

  TEST_CASE("TaskTarget: tasks left when it goes are destroyed unrun", "[Threading]")
  {
    bool ran = false;
    bool destroyed = false;
    {
      TaskTarget target;
      target.Post([&ran, flag = CreateUnique<DestructionFlag>(destroyed)]() { ran = true; });
    }
    CHECK_FALSE(ran);
    CHECK(destroyed);
  }
}

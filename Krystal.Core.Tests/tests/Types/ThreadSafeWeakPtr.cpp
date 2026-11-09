#include "Krystal.Core/Types/ThreadSafeWeakPtr.hpp"
#include "catch2/catch_test_macros.hpp"
#include <atomic>
#include <thread>
#include <vector>

namespace krys::tests
{
  namespace
  {
    class Shared final : public RefCountedThreadSafeAndCanMakeWeakPtrThreadSafe<Shared>
    {
    private:
      std::atomic<int> *_destroyed;

    public:
      explicit Shared(std::atomic<int> &destroyed) noexcept : _destroyed(&destroyed)
      {
      }

      ~Shared() noexcept
      {
        ++*_destroyed;
      }
    };

    constexpr int ThreadCount = 4;
    constexpr int Iterations = 10'000;

    /// @brief Runs @p work on several threads at once and waits for them.
    template <typename F>
    void OnThreads(F work)
    {
      std::vector<std::thread> threads;
      for (int i = 0; i < ThreadCount; ++i)
      {
        threads.emplace_back(work);
      }
      for (auto &thread : threads)
      {
        thread.join();
      }
    }
  }

  TEST_CASE("ThreadSafeWeakPtr: strong references from several threads", "[ThreadSafeWeakPtr]")
  {
    std::atomic<int> destroyed {0};
    RefPtr<Shared> strong = CreateRef<Shared>(destroyed);

    SECTION("Without a weak pointer, the count stays in the object")
    {
      OnThreads(
        [&strong]()
        {
          for (int i = 0; i < Iterations; ++i)
          {
            RefPtr<Shared> copy = strong;
          }
        });
      CHECK(strong->GetRefCount() == 1u);
    }

    SECTION("Through a weak pointer, which moves the count into a control block")
    {
      ThreadSafeWeakPtr<Shared> weak {strong};
      std::atomic<bool> lost {false};
      OnThreads(
        [&weak, &lost]()
        {
          for (int i = 0; i < Iterations; ++i)
          {
            if (!weak.get())
            {
              lost = true;
            }
          }
        });
      CHECK_FALSE(lost);
      CHECK(strong->GetRefCount() == 1u);
      CHECK(weak.get() == strong);
    }

    strong = nullptr;
    CHECK(destroyed == 1);
  }

  TEST_CASE("ThreadSafeWeakPtr: empties when the last strong reference goes", "[ThreadSafeWeakPtr]")
  {
    std::atomic<int> destroyed {0};
    RefPtr<Shared> strong = CreateRef<Shared>(destroyed);
    ThreadSafeWeakPtr<Shared> weak {strong};
    ThreadSafeWeakPtr<Shared> copy = weak;

    SECTION("On this thread")
    {
      strong = nullptr;
    }

    SECTION("On another thread")
    {
      std::thread thread([moved = krys::move(strong)]() mutable { moved = nullptr; });
      thread.join();
    }

    CHECK(destroyed == 1);
    CHECK_FALSE(weak.get());
    CHECK_FALSE(copy.get());
  }

  TEST_CASE("ThreadSafeWeakPtr: a weak pointer can outlive the object", "[ThreadSafeWeakPtr]")
  {
    std::atomic<int> destroyed {0};
    ThreadSafeWeakPtr<Shared> weak;
    {
      RefPtr<Shared> strong = CreateRef<Shared>(destroyed);
      weak = strong;
      CHECK(weak.get() == strong);
    }
    CHECK(destroyed == 1);
    CHECK_FALSE(weak.get());

    // Clearing it releases the control block.
    weak = nullptr;
    CHECK_FALSE(weak.get());
  }
}

#include "Krystal.Core/Threading/Thread.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include <atomic>
#include <thread>

namespace krys::tests
{
  TEST_CASE("Thread: runs its body on a thread of its own, and waits for it", "[Threading]")
  {
    std::atomic<bool> ran {false};
    std::thread::id bodyThread;
    auto owned = CreateUnique<int>(7);
    std::thread::id threadId;
    {
      // The body can own what can only be moved.
      Thread thread(u8"Test thread",
                    [&, owned = krys::move(owned)]()
                    {
                      bodyThread = std::this_thread::get_id();
                      ran = *owned == 7;
                    });
      CHECK(thread.Name() == u8"Test thread");
      threadId = thread.Id();
    }
    // Destruction waited for the body.
    CHECK(ran);
    CHECK(bodyThread == threadId);
    CHECK(bodyThread != std::this_thread::get_id());
  }
}

#include "Krystal.Threading/MainThread.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Threading/Thread.hpp"
#include <atomic>

namespace krys::threads::tests
{
  TEST_CASE("IsMainThread: only the thread the program started on", "[Threading]")
  {
    // Catch2 runs the tests on the thread `main` runs on.
    CHECK(IsMainThread());

    std::atomic<bool> onMain {true};
    {
      Thread thread(u8"Other", [&onMain]() { onMain = IsMainThread(); });
    }
    CHECK_FALSE(onMain);
  }
}

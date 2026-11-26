#include "Krystal.Core/Threading/MainThread.hpp"
#include "catch2/catch_test_macros.hpp"
#include <thread>

namespace krys::tests
{
  TEST_CASE("IsMainThread: only the thread the program started on", "[Threading]")
  {
    // Catch2 runs the tests on the thread `main` runs on.
    CHECK(IsMainThread());

    bool onMain = true;
    std::thread thread([&onMain]() { onMain = IsMainThread(); });
    thread.join();
    CHECK_FALSE(onMain);
  }
}

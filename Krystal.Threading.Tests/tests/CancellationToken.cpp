#include "Krystal.Threading/CancellationToken.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Threading/Thread.hpp"

namespace krys::tests
{
  TEST_CASE("CancellationToken: another thread sees the cancellation", "[Threading]")
  {
    auto token = CancellationToken::Create();
    CHECK_FALSE(token->IsCancelled());
    {
      Thread thread(u8"Canceller", [token]() { token->Cancel(); });
    }
    CHECK(token->IsCancelled());
  }
}

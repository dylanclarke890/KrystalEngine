#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Types/Exception.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("AbortSignal: timeout()", "[DOM][AbortSignal]")
  {
    using namespace std::chrono_literals;
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();

    Ref<AbortSignal> signal = AbortSignal::Timeout(100);
    CHECK_FALSE(signal->Aborted());

    loop.Run(loop.Now() + 99ms);
    CHECK_FALSE(signal->Aborted());

    // After the timeout, a task on the timer task source aborts it with a "TimeoutError".
    loop.Run(loop.Now() + 1ms);
    REQUIRE(signal->Aborted());
    auto *reason = std::any_cast<Exception>(&signal->Reason());
    REQUIRE(reason);
    CHECK(reason->Code() == ExceptionCode::TimeoutError);
  }
}

#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortController.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
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

  TEST_CASE("AbortSignal: any() and its dependents' lifetime", "[DOM][AbortSignal]")
  {
    AbortController controller;
    auto &source = *controller.Signal();

    SECTION("A dependent with an abort listener lives while its source can abort it")
    {
      bool aborted = false;
      EventListenerHandle handle;
      WeakPtr<AbortSignal> weakDependent;
      {
        auto dependent = AbortSignal::Any({ShareRef(source)});
        weakDependent = CreateWeakPtr(dependent.get());
        handle =
          dependent->AddEventListener(DOMStringAtom(EventNames::Abort), [&](Event &) { aborted = true; });
      }
      REQUIRE(weakDependent);
      controller.Abort();
      CHECK(aborted);
      html::EventLoop::Current().RunUntilIdle();
    }

    SECTION("A dependent with nothing to run is freed once dropped")
    {
      WeakPtr<AbortSignal> weakDependent;
      {
        auto dependent = AbortSignal::Any({ShareRef(source)});
        weakDependent = CreateWeakPtr(dependent.get());
      }
      CHECK_FALSE(weakDependent);
      // Aborting the source skips the dependent that's gone.
      controller.Abort();
      CHECK(source.Aborted());
    }

    SECTION("Removing its listener lets it be freed")
    {
      WeakPtr<AbortSignal> weakDependent;
      {
        auto dependent = AbortSignal::Any({ShareRef(source)});
        weakDependent = CreateWeakPtr(dependent.get());
        auto handle = dependent->AddEventListener(DOMStringAtom(EventNames::Abort), [](Event &) {});
      }
      html::EventLoop::Current().RunUntilIdle();
      CHECK_FALSE(weakDependent);
    }

    SECTION("A dependent of a dependent follows the first one's sources")
    {
      auto inner = AbortSignal::Any({ShareRef(source)});
      auto outer = AbortSignal::Any({inner});
      CHECK_FALSE(outer->Aborted());
      controller.Abort();
      CHECK(inner->Aborted());
      CHECK(outer->Aborted());
    }
  }
}

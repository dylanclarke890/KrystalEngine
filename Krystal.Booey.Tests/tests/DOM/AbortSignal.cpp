#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortController.hpp"
#include "Krystal.Booey/DOM/Algorithms/AbortAlgorithms.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/Types/Exception.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE(
    "AbortSignal.abort(): an aborted signal with the reason, or an AbortError",
    "[DOM][AbortSignal][dom:dom-abortsignal-abort][dom:dom-abortsignal-reason][dom:abortsignal-abort-reason]")
  {
    SECTION("Without a reason")
    {
      auto signal = AbortSignal::Abort();
      REQUIRE(signal->Aborted());
      auto *reason = std::any_cast<Exception>(&signal->Reason());
      REQUIRE(reason);
      CHECK(reason->Code() == ExceptionCode::AbortError);
    }

    SECTION("With a reason")
    {
      auto signal = AbortSignal::Abort(Any(42));
      REQUIRE(signal->Aborted());
      CHECK(std::any_cast<int>(signal->Reason()) == 42);
    }

    SECTION("An empty reason (undefined) counts as not given")
    {
      auto signal = AbortSignal::Abort(Any());
      REQUIRE(signal->Aborted());
      auto *reason = std::any_cast<Exception>(&signal->Reason());
      REQUIRE(reason);
      CHECK(reason->Code() == ExceptionCode::AbortError);
    }
  }

  TEST_CASE("AbortSignal.timeout(): aborts with a TimeoutError after the timeout",
            "[DOM][AbortSignal][dom:dom-abortsignal-timeout]")
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

  TEST_CASE("AbortSignal.any(): a signal aborted with the reason of whichever signal aborts first",
            "[DOM][AbortSignal][dom:dom-abortsignal-any][dom:create-a-dependent-abort-signal]"
            "[dom:abortsignal-dependent][dom:abortsignal-source-signals][dom:abortsignal-dependent-signals]")
  {
    AbortController first;
    AbortController second;

    SECTION("Already aborted if one of the signals is, with the first aborted one's reason")
    {
      auto signal =
        AbortSignal::Any({ShareRef(*first.Signal()), AbortSignal::Abort(Any(1)), AbortSignal::Abort(Any(2))});
      REQUIRE(signal->Aborted());
      CHECK(std::any_cast<int>(signal->Reason()) == 1);
    }

    SECTION("With no signals, it never aborts")
    {
      auto signal = AbortSignal::Any({});
      CHECK_FALSE(signal->Aborted());
    }

    SECTION("Aborts with the reason of the signal that aborts")
    {
      auto signal = AbortSignal::Any({ShareRef(*first.Signal()), ShareRef(*second.Signal())});
      CHECK_FALSE(signal->Aborted());

      second.Abort(Any(2));
      REQUIRE(signal->Aborted());
      CHECK(std::any_cast<int>(signal->Reason()) == 2);

      first.Abort(Any(1));
      CHECK(std::any_cast<int>(signal->Reason()) == 2);
    }

    SECTION("A signal given twice is a source once")
    {
      auto signal = AbortSignal::Any({ShareRef(*first.Signal()), ShareRef(*first.Signal())});
      int aborts = 0;
      auto listener = signal->AddEventListener(EventNames::Abort, [&](Event &) { ++aborts; });
      first.Abort();
      CHECK(aborts == 1);
    }

    SECTION("A dependent of a dependent follows the first one's sources")
    {
      auto inner = AbortSignal::Any({ShareRef(*first.Signal())});
      auto outer = AbortSignal::Any({inner});
      CHECK_FALSE(outer->Aborted());
      first.Abort(Any(3));
      CHECK(inner->Aborted());
      REQUIRE(outer->Aborted());
      CHECK(std::any_cast<int>(outer->Reason()) == 3);
    }
  }

  TEST_CASE("AbortSignal.aborted and throwIfAborted(): whether the signal has an abort reason",
            "[DOM][AbortSignal][dom:dom-abortsignal-aborted][dom:abortsignal-aborted]"
            "[dom:dom-abortsignal-throwifaborted]")
  {
    AbortController controller;
    auto &signal = *controller.Signal();
    CHECK_FALSE(signal.Aborted());
    CHECK_FALSE(signal.Reason().has_value());
    CHECK_NOTHROW(signal.ThrowIfAborted());

    controller.Abort(Any(5));
    CHECK(signal.Aborted());

    bool thrown = false;
    try
    {
      signal.ThrowIfAborted();
    }
    catch (const krys::Any &reason)
    {
      thrown = true;
      CHECK(std::any_cast<int>(reason) == 5);
    }
    CHECK(thrown);
  }

  TEST_CASE("AbortSignal.onabort: an event handler for the abort event",
            "[DOM][AbortSignal][dom:dom-abortsignal-onabort][dom:abortsignal-onabort]"
            "[dom:eventdef-abortsignal-abort]")
  {
    AbortController controller;
    auto &signal = *controller.Signal();
    CHECK_FALSE(signal.OnAbort());

    int calls = 0;
    signal.SetOnAbort(
      [&](Event &event)
      {
        ++calls;
        CHECK(event.Type() == EventNames::Abort);
        CHECK(event.IsTrusted());
        CHECK_FALSE(event.Bubbles());
        CHECK_FALSE(event.Cancelable());
      });
    CHECK(signal.OnAbort());

    controller.Abort();
    controller.Abort();
    CHECK(calls == 1);
  }

  TEST_CASE("Garbage collection: a dependent signal lives while its sources can abort it",
            "[DOM][AbortSignal][dom:abort-signal-garbage-collection]")
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

      // Once aborted, it's let go.
      html::EventLoop::Current().RunUntilIdle();
      CHECK_FALSE(weakDependent);
    }

    SECTION("An abort event handler keeps it alive too")
    {
      bool aborted = false;
      WeakPtr<AbortSignal> weakDependent;
      {
        auto dependent = AbortSignal::Any({ShareRef(source)});
        weakDependent = CreateWeakPtr(dependent.get());
        dependent->SetOnAbort([&](Event &) { aborted = true; });
      }
      REQUIRE(weakDependent);
      controller.Abort();
      CHECK(aborted);
      html::EventLoop::Current().RunUntilIdle();
    }

    SECTION("An abort algorithm keeps it alive, until it's removed")
    {
      bool aborted = false;
      AbortAlgorithmCallback algorithm = [&](const Any &)
      {
        aborted = true;
      };
      WeakPtr<AbortSignal> weakDependent;
      {
        auto dependent = AbortSignal::Any({ShareRef(source)});
        weakDependent = CreateWeakPtr(dependent.get());
        AbortAlgorithms::Add(algorithm, *dependent);
      }
      REQUIRE(weakDependent);

      SECTION("Aborting runs it")
      {
        controller.Abort();
        CHECK(aborted);
      }

      SECTION("Removing it lets the signal be freed")
      {
        AbortAlgorithms::Remove(algorithm, *weakDependent);
        html::EventLoop::Current().RunUntilIdle();
        CHECK_FALSE(weakDependent);
      }
    }

    SECTION("A listener doesn't keep it alive once its sources are gone")
    {
      WeakPtr<AbortSignal> weakDependent;
      EventListenerHandle handle;
      {
        AbortController other;
        auto dependent = AbortSignal::Any({ShareRef(*other.Signal())});
        weakDependent = CreateWeakPtr(dependent.get());
        handle = dependent->AddEventListener(DOMStringAtom(EventNames::Abort), [](Event &) {});
      }
      html::EventLoop::Current().RunUntilIdle();
      CHECK_FALSE(weakDependent);
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
  }
}

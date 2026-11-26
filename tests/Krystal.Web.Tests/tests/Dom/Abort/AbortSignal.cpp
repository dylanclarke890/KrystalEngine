#include "Krystal.Web/Dom/Abort/AbortSignal.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web/Dom/Abort/AbortAlgorithms.hpp"
#include "Krystal.Web/Dom/Abort/AbortController.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"
#include "Krystal.Web/Dom/Events/EventListenerHandle.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Types/Exception.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"

namespace krys::web::dom::tests
{
  TEST_CASE(
    "AbortSignal.abort(): an aborted signal with the reason, or an AbortError",
    "[DOM][AbortSignal][dom:dom-abortsignal-abort][dom:dom-abortsignal-reason][dom:abortsignal-abort-reason]")
  {
    SECTION("Without a reason")
    {
      auto signal = AbortSignal::Abort();
      REQUIRE(signal->Aborted());
      const auto &reason = signal->Reason();
      REQUIRE(reason);
      CHECK(reason->Code() == ExceptionCode::AbortError);
    }

    SECTION("With a reason")
    {
      auto signal = AbortSignal::Abort(Exception(ExceptionCode::AbortError, u8"42"));
      REQUIRE(signal->Aborted());
      CHECK(signal->Reason()->Message() == u8"42");
    }
  }

  TEST_CASE("AbortSignal.timeout(): aborts with a TimeoutError after the timeout",
            "[DOM][AbortSignal][dom:dom-abortsignal-timeout]")
  {
    using namespace std::chrono_literals;
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();

    auto document = CreateRef<Document>();
    Ref<AbortSignal> signal = AbortSignal::Timeout(*document, 100);
    CHECK_FALSE(signal->Aborted());

    loop.Run(loop.Now() + 99ms);
    CHECK_FALSE(signal->Aborted());

    // After the timeout, a task on the timer task source aborts it with a "TimeoutError".
    loop.Run(loop.Now() + 1ms);
    REQUIRE(signal->Aborted());
    const auto &reason = signal->Reason();
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
      auto signal = AbortSignal::Any({ShareRef(first.Signal()),
                                      AbortSignal::Abort(Exception(ExceptionCode::AbortError, u8"1")),
                                      AbortSignal::Abort(Exception(ExceptionCode::AbortError, u8"2"))});
      REQUIRE(signal->Aborted());
      CHECK(signal->Reason()->Message() == u8"1");
    }

    SECTION("With no signals, it never aborts")
    {
      auto signal = AbortSignal::Any({});
      CHECK_FALSE(signal->Aborted());
    }

    SECTION("Aborts with the reason of the signal that aborts")
    {
      auto signal = AbortSignal::Any({ShareRef(first.Signal()), ShareRef(second.Signal())});
      CHECK_FALSE(signal->Aborted());

      second.Abort(Exception(ExceptionCode::AbortError, u8"2"));
      REQUIRE(signal->Aborted());
      CHECK(signal->Reason()->Message() == u8"2");

      first.Abort(Exception(ExceptionCode::AbortError, u8"1"));
      CHECK(signal->Reason()->Message() == u8"2");
    }

    SECTION("A signal given twice is a source once")
    {
      auto signal = AbortSignal::Any({ShareRef(first.Signal()), ShareRef(first.Signal())});
      int aborts = 0;
      auto listener = signal->AddEventListener(event_names::Abort, [&](Event &) { ++aborts; });
      first.Abort();
      CHECK(aborts == 1);
    }

    SECTION("A dependent of a dependent follows the first one's sources")
    {
      auto inner = AbortSignal::Any({ShareRef(first.Signal())});
      auto outer = AbortSignal::Any({inner});
      CHECK_FALSE(outer->Aborted());
      first.Abort(Exception(ExceptionCode::AbortError, u8"3"));
      CHECK(inner->Aborted());
      REQUIRE(outer->Aborted());
      CHECK(outer->Reason()->Message() == u8"3");
    }
  }

  TEST_CASE("AbortSignal.aborted and throwIfAborted(): whether the signal has an abort reason",
            "[DOM][AbortSignal][dom:dom-abortsignal-aborted][dom:abortsignal-aborted]"
            "[dom:dom-abortsignal-throwifaborted]")
  {
    AbortController controller;
    auto &signal = controller.Signal();
    CHECK_FALSE(signal.Aborted());
    CHECK_FALSE(signal.Reason().has_value());
    CHECK_FALSE(signal.ThrowIfAborted().HasException());

    controller.Abort(Exception(ExceptionCode::AbortError, u8"5"));
    CHECK(signal.Aborted());

    auto result = signal.ThrowIfAborted();
    REQUIRE(result.HasException());
    CHECK(result.GetException().Message() == u8"5");
  }

  TEST_CASE("AbortSignal.onabort: an event handler for the abort event",
            "[DOM][AbortSignal][dom:dom-abortsignal-onabort][dom:abortsignal-onabort]"
            "[dom:eventdef-abortsignal-abort]")
  {
    AbortController controller;
    auto &signal = controller.Signal();
    CHECK_FALSE(signal.OnAbort());

    int calls = 0;
    signal.SetOnAbort(
      [&](Event &event)
      {
        ++calls;
        CHECK(event.Type() == event_names::Abort);
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
    auto &source = controller.Signal();

    SECTION("A dependent with an abort listener lives while its source can abort it")
    {
      bool aborted = false;
      EventListenerHandle handle;
      WeakPtr<AbortSignal> weakDependent;
      {
        auto dependent = AbortSignal::Any({ShareRef(source)});
        weakDependent = CreateWeakPtr(dependent.get());
        handle = dependent->AddEventListener(event_names::Abort, [&](Event &) { aborted = true; });
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
      AbortAlgorithmHandle algorithm;
      WeakPtr<AbortSignal> weakDependent;
      {
        auto dependent = AbortSignal::Any({ShareRef(source)});
        weakDependent = CreateWeakPtr(dependent.get());
        algorithm = AbortAlgorithms::Add([&](const Exception &) { aborted = true; }, *dependent);
      }
      REQUIRE(weakDependent);

      SECTION("Aborting runs it")
      {
        controller.Abort();
        CHECK(aborted);
      }

      SECTION("Removing it lets the signal be freed")
      {
        algorithm.Remove();
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
        auto dependent = AbortSignal::Any({ShareRef(other.Signal())});
        weakDependent = CreateWeakPtr(dependent.get());
        handle = dependent->AddEventListener(event_names::Abort, [](Event &) {});
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
        auto handle = dependent->AddEventListener(event_names::Abort, [](Event &) {});
      }
      html::EventLoop::Current().RunUntilIdle();
      CHECK_FALSE(weakDependent);
    }

    SECTION("A listener's abort steps keep it alive, until the listener is removed")
    {
      auto target = CreateRef<EventTarget>();
      WeakPtr<AbortSignal> weakDependent;
      EventListenerHandle listener;
      {
        auto dependent = AbortSignal::Any({ShareRef(source)});
        weakDependent = CreateWeakPtr(dependent.get());
        AddEventListenerOptions options;
        options.Signal = dependent.get();
        listener = target->AddEventListener(event_names::Load, [](Event &) {}, options);
      }
      REQUIRE(weakDependent);

      // The listener holds its signal, and removing it takes its abort steps off the signal too.
      listener.Remove();
      html::EventLoop::Current().RunUntilIdle();
      CHECK_FALSE(weakDependent);
    }
  }

  TEST_CASE("Aborting ongoing activities: the introduction's example",
            "[DOM][AbortSignal][dom:aborting-ongoing-activities]")
  {
    // An API that takes a signal checks it before it starts, and stops with its reason when it's aborted.
    struct Amazingness
    {
      Maybe<Exception> RejectedWith;
      EventListenerHandle Listener;

      ExceptionOr<void> Start(AbortSignal &signal)
      {
        if (auto result = signal.ThrowIfAborted(); result.HasException())
        {
          return result;
        }
        Listener = signal.AddEventListener(event_names::Abort,
                                           [this, &signal](Event &) { RejectedWith = signal.Reason(); });
        return {};
      }
    };

    AbortController controller;
    Amazingness amazingness;
    REQUIRE_FALSE(amazingness.Start(controller.Signal()).HasException());
    CHECK_FALSE(amazingness.RejectedWith.has_value());

    controller.Abort(Exception(ExceptionCode::AbortError, u8"42"));
    REQUIRE(amazingness.RejectedWith);
    CHECK(amazingness.RejectedWith->Message() == u8"42");

    // Started after the abort, it fails with the reason straight away.
    Amazingness late;
    auto result = late.Start(controller.Signal());
    REQUIRE(result.HasException());
    CHECK(result.GetException().Message() == u8"42");
  }
}
